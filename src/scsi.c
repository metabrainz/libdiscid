/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


--------------------------------------------------------------------------- */

/*
 * These commands are standard SCSI commands defined by INCITS T10
 * SPC: primary commands
 * MMC: multimedia commands
 *
 * The specs can be found on the net (t10.org, needs registration)
 * The Seagate SCSI Commands Reference Manual is also an
 * excellant resource that is freely available.
 */

#include <stdio.h>
#include <string.h>

#include "discid/discid_private.h"
#include "scsi.h"

#define BYTES_SECTOR 2448	/* including raw (audio) data */
#define SUBCHANNEL_BYTES 96	/* bytes with sub-channel info (per sector) */
#define BYTES_RAW (2448 - 96) 	/* without sub-channel */
/* each sub-channel byte includes 1 bit for each of the subchannel types */
#define BITS_SUBCHANNEL 96


/* Send a scsi command and receive data. */
static int scsi_cmd(int fd, unsigned char *cmd, int cmd_len,
	     unsigned char *data, int data_len) {
	return mb_scsi_cmd_unportable(fd, cmd, cmd_len, data, data_len);
}

/* This uses CRC-16 (CRC-CCITT) as defined for audio CDs
 * to check the parity for 10*8 = 80 data bits
 * using 2*8 = 16 checksum/parity bits.
 * We feed the complete Q-channel data, data+parity (96 bits), to the algorithm
 * and check for a remainder of 0.
 */
static int check_crc(const unsigned char data[10], const unsigned char crc[2]) {
	/* The generator polynomial to be used:
	 * x^16+x^12+x^5 = 10001000000100001 = 0x11021
	 * so the degree is 16 and the generator has 17 bits */
	const long generator = 0x11021;
	long remainder = 0;	/* start value */
	int data_bit = 0;
	int byte_num;
	unsigned char data_byte, mask;

	do {
		/* fill the remainder with data until the 17th bit is one */
		while (data_bit < BITS_SUBCHANNEL && remainder >> 16 == 0) {
			byte_num = data_bit >> 3;
			if (byte_num < 10) {
				data_byte = data[byte_num];
			} else {
				/* crc/parity stored inverted on disc */
				data_byte = ~crc[byte_num-10];
			}
			remainder = (remainder << 1) ;
			/* add the current bit from the current data_byte */
			mask = 0x80 >> (data_bit % 8);
			remainder += (data_byte & mask) != 0x0;
			data_bit++;
		}

		/* We do a polynomial division by the generator modulo(2).
		 * So we get the (new) remainder with XOR (^)
		 * and don't care about the actual result of the division.
		 */
		if (remainder >> 16 == 1)
			remainder = remainder ^ generator;

	} while (data_bit < BITS_SUBCHANNEL); /* while data left */

	return remainder == 0;
}

static int decode_isrc(unsigned char *q_channel, char *isrc) {
	int isrc_pos;
	int data_pos;
	int bit_pos;
	int bit;
	unsigned char buffer;
	unsigned char crc[2];

	isrc_pos = 0;
	/* upper 4 bits of q_channel[0] are CONTROL */
	/* lower 4 bits of q_channel[0] = ADR = 0x03 -> mode 3 -> ISRC data */
	data_pos = 1;
	bit_pos = 7;
	buffer = 0;
	/* first 5 chars of ISRC are alphanumeric and 6-bit BCD encoded */
	for (bit = 0; bit < 5 * 6; bit++) {
		buffer = buffer << 1;
		if ((q_channel[data_pos] & (1 << bit_pos)) == (1 << bit_pos)) {
			buffer++;
		}
		bit_pos--;
		if ((bit + 1) % 8 == 0) {
			bit_pos = 7;
			data_pos++;
		}
		if ((bit + 1) % 6 == 0) {
			/* 0x3f = only lowest 6 bits set */
			isrc[isrc_pos] = '0' + (buffer & 0x3f);
			isrc_pos++;
			buffer = 0;
		}
	}
	/* buffer[4] includes 2 zero bits
	 * last 7 chars of ISRC are only numeric and 4-bit encoded
	 */
	isrc[5] = '0' + (q_channel[5] >> 4);
	isrc[6] = '0' + (q_channel[5] & 0x0f);
	isrc[7] = '0' + (q_channel[6] >> 4);
	isrc[8] = '0' + (q_channel[6] & 0x0f);
	isrc[9] = '0' + (q_channel[7] >> 4);
	isrc[10] = '0' + (q_channel[7] & 0x0f);
	isrc[11] = '0' + (q_channel[8] >> 4);
	/* q_channel[8] & 0x0f are zero bits */
	/* q_channel[9] is AFRAME */
	crc[0] = q_channel[10];
	crc[1] = q_channel[11];

	if (!check_crc(q_channel, crc)) {
		return 0;
	} else {
		return 1;
	}
}


void mb_scsi_stop_disc(int fd) {
	unsigned char cmd[6];

	memset(cmd, 0, sizeof cmd);

	cmd[0] = 0x1b;		/* START STOP UNIT (SPC)*/
	cmd[1] = 1;		/* return immediately */
	/* cmd 2-3 reserved */
	cmd[4] = 0;		/* stop */
	/* cmd 5 = control byte */

	if (scsi_cmd(fd, cmd, sizeof cmd, NULL, 0) != 0) {
		fprintf(stderr, "Warning: Cannot stop device");
	}
}

void mb_scsi_read_track_isrc(int fd, mb_disc_private *disc, int track_num) {
	int i;
	unsigned char cmd[10];
	unsigned char data[24];
	char buffer[ISRC_STR_LENGTH+1];

	memset(cmd, 0, sizeof cmd);
	memset(data, 0, sizeof data);
	memset(buffer, 0, sizeof buffer);

	cmd[0] = 0x42;		/* READ SUB-CHANNEL (MMC)*/
	/* cmd[1] reserved / MSF bit (unused) */
	cmd[2] = 1 << 6;	/* 6th bit set (SUBQ) -> get sub-channel data */
	cmd[3] = 0x03;		/* get ISRC (ADR 3, Q sub-channel Mode-3) */
	/* 4+5 reserved */
	cmd[6] = track_num;
	/* cmd[7] = upper byte of the transfer length */
	cmd[8] = sizeof data;  /* transfer length in bytes (4 header, 20 data)*/
	/* cmd[9] = control byte */

	if (scsi_cmd(fd, cmd, sizeof cmd, data, sizeof data) != 0) {
		fprintf(stderr, "Warning: Cannot get ISRC code for track %d\n",
			track_num);
		return;
	}

	/* data[1:4] = sub-q channel data header (audio status, data length) */
	if (data[8] & (1 << 7)) { /* TCVAL is set -> ISRCs valid */
		for (i = 0; i < ISRC_STR_LENGTH; i++) {
			buffer[i] = data[9 + i];
		}
		buffer[ISRC_STR_LENGTH] = 0;
		strncpy(disc->isrc[track_num], buffer, ISRC_STR_LENGTH);
	}
	/* data[21:23] = zero, AFRAME, reserved */

}

void mb_scsi_read_track_isrc_raw(int fd, mb_disc_private *disc, int track_num) {
	/* There should be at least one ISRC in 100 sectors acc to spec.
	 * 279 seems to be the maximum we can request without failing,
	 * which is close to 4 seconds and contains up to 3 ISRCs.
	 * We break when we found one with valid CRC.
	 */
	const int SECTORS_TO_CHECK = 279;
	int num_sectors, max_sectors;
	int disc_offset;		/* in sectors */
	int data_len, data_offset;	/* in bytes */
	unsigned char *data;	/* overall data */
	unsigned char cmd[12];
	unsigned char q_buffer;
	unsigned char q_data[12]; /* sub-channel data in 1 sector */
	char isrc[ISRC_STR_LENGTH+1];
	int char_num;
	int sector;
	int i;
	int isrc_found = 0;
	int warning_shown = 0;

	memset(cmd, 0, sizeof cmd);
	memset(isrc, 0, sizeof isrc);

	/* allocate memory for the amount of sectors we would like to read */
	max_sectors = discid_get_track_length((DiscId) disc, track_num);
	if (max_sectors > SECTORS_TO_CHECK)
		num_sectors = SECTORS_TO_CHECK;
	else
		num_sectors = max_sectors;
	data_len = num_sectors * BYTES_SECTOR;
	data = (unsigned char *) calloc(data_len, 1);

	disc_offset = disc->track_offsets[track_num];

	/* 0xbe = READ CD, implementation optional in contrast to 0x42
	 * support given with GET CONFIGURATION (0x46)
	 * Support part of:
	 * Multi-Read Feature (0x001d) and
	 * CD Read Feature (0x001e),
	 */
	cmd[0] = 0xbe;	/* READ CD (MMC) */
	cmd[2] = disc_offset >> 24;
	cmd[3] = disc_offset >> 16;
	cmd[4] = disc_offset >> 8;
	cmd[5] = disc_offset;  /* from where to start reading */
	cmd[6] = num_sectors >> 16;
	cmd[7] = num_sectors >> 8;
	cmd[8] = num_sectors; /* sectors to read */
	cmd[9] = 0xF8; 	/* 11111000 sync=1 all-header=11 user=1, ecc=1
			 * no-error=00 */
	cmd[10] = 0x01; 	/* Sub-Channel Selection raw P-W=001*/
	/* cmd[11] = control byte */

	if (scsi_cmd(fd, cmd, sizeof cmd, data, data_len) != 0) {
		fprintf(stderr, "Warning: Cannot get ISRC code for track %d\n",
			track_num);
		return;
	}

	for (sector = 0; sector < num_sectors; sector++) {
		char_num = 0;
		memset(q_data, 0, sizeof q_data);
		q_buffer = 0;

		/* The first BYTES_RAW bytes have only raw data,
		 * but the following SUBCHANNEL_BYTES
		 * include sub-channel information.
		 * Every one of these bytes includes one bit
		 * for every sub-channel type.
		 * We skip ahead to these, fetch the bits for channel Q
		 * and check if there is ISRC information in them.
		 */
		for (i = BYTES_RAW; i < BYTES_SECTOR; i++) {
			q_buffer = q_buffer << 1;
			data_offset = i + (sector * BYTES_SECTOR);

			/* the 6th bit is the Q-channel bit
			 * we want to collect these bits
			 */
			if ((data[data_offset] & (1 << 6)) != 0x0) {
				q_buffer++;
			}

			if ((i + 1) % 8 == 0) {
				/* we have gathered one complete byte */
				q_data[char_num] = q_buffer;
				if (char_num == 0) {
					/* test if we got the right type
					 * of q-channel (ADR = 0x03)
					 * upper 4 bit are CONTROL
					 * Go to next sector otherwise
					 */
					if ((q_buffer & 0x0F) != 0x03) {
						break;
					}
				}
				char_num++;
				q_buffer = 0;
			}
		}
		if ((q_data[0] & 0x0F) == 0x03) {
			/* We found a Q-channel with ISRC data.
			 * Test if the CRC matches and stop searching if so.
			 */
			if(decode_isrc(q_data, isrc)) {
				isrc_found = 1;
				break;
			} else {
				fprintf(stderr,
					"Warning: CRC mismatch track %d: %s\n",
					track_num, isrc);
				warning_shown = 1;
			}
		}
	}
	if (isrc_found) {
		if (warning_shown) {
			fprintf(stderr,
				"         valid ISRC for track %d: %s\n",
				track_num, isrc);
		}
		strncpy(disc->isrc[track_num], isrc, ISRC_STR_LENGTH);
	}
}


/* EOF */
