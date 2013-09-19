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
 * resource that is freely available, but only for primary commands.
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "discid/discid.h"
#include "discid/discid_private.h"
#include "scsi.h"

#define SUBCHANNEL_BYTES 96	/* bytes with sub-channel info (per sector) */
/* each sub-channel byte includes 1 bit for each of the subchannel types */
#define BITS_SUBCHANNEL 96

enum isrc_search {
	NOTHING_FOUND = 0,
	ISRC_FOUND = 1,
	CRC_MISMATCH = 2,
};


/* Send a scsi command and receive data. */
static int scsi_cmd(mb_scsi_handle handle, unsigned char *cmd, int cmd_len,
	     unsigned char *data, int data_len) {
	return mb_scsi_cmd_unportable(handle, cmd, cmd_len, data, data_len);
}

/* This uses CRC-16 (CRC-CCITT?) as defined for audio CDs
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

static enum isrc_search find_isrc_in_sector(unsigned char *data, char *isrc) {
	unsigned char q_buffer;
	unsigned char q_data[12];	/* sub-channel data in 1 sector */
	int char_num;
	int i;

	char_num = 0;
	memset(q_data, 0, sizeof q_data);
	q_buffer = 0;

	/* Every one of these SUBCHANNEL_BYTES includes one bit
	 * for every sub-channel type.
	 * We fetch the bits for channel Q
	 * and check if there is ISRC information in them.
	 */
	for (i = 0; i < SUBCHANNEL_BYTES; i++) {
		q_buffer = q_buffer << 1;

		/* the 6th bit is the Q-channel bit
		 * we want to collect these bits
		 */
		if ((data[i] & (1 << 6)) != 0x0) {
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
			return ISRC_FOUND;
		} else {
			return CRC_MISMATCH;
		}
	} else {
		return NOTHING_FOUND;
	}
}

void mb_scsi_stop_disc(mb_scsi_handle handle) {
	unsigned char cmd[6];

	memset(cmd, 0, sizeof cmd);

	cmd[0] = 0x1b;		/* START STOP UNIT (SPC)*/
	cmd[1] = 1;		/* return immediately */
	/* cmd 2-3 reserved */
	cmd[4] = 0;		/* stop */
	/* cmd 5 = control byte */

	if (scsi_cmd(handle, cmd, sizeof cmd, NULL, 0) != 0) {
		fprintf(stderr, "Warning: Cannot stop device");
	}
}

void mb_scsi_read_track_isrc(mb_scsi_handle handle, mb_disc_private *disc,
			     int track_num) {
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

	if (scsi_cmd(handle, cmd, sizeof cmd, data, sizeof data) != 0) {
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

/* We read sectors from a track until finding a valid ISRC.
 * An empty ISRC is valid in that context -> leads to empty string.
 * Up to 100 sectors have to be read for every ISRC candidate.
 */
void mb_scsi_read_track_isrc_raw(mb_scsi_handle handle, mb_disc_private *disc,
				 int track_num) {
	int max_sectors;
	int disc_offset;	/* in sectors */
	int data_len;		/* in bytes */
	unsigned char *data;	/* overall data */
	unsigned char cmd[12];
	char isrc[ISRC_STR_LENGTH+1];
	int sector = 0;
	enum isrc_search search_result;
	int isrc_found = 0;
	int warning_shown = 0;

	data_len = SUBCHANNEL_BYTES;
	data = (unsigned char *) calloc(data_len, 1);

	/* start reading sectors at start of track */
	disc_offset = disc->track_offsets[track_num];
	max_sectors = mb_disc_get_track_length(disc, track_num);

	while (!isrc_found && sector <= max_sectors) {
		memset(cmd, 0, sizeof cmd);
		memset(isrc, 0, sizeof isrc);
		memset(data, 0, data_len);

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
		cmd[5] = disc_offset;	/* from where to start reading */
		/* cmd[6] and cmd[7] are unused upper bytes of sector count */
		cmd[8] = 1;		/* sectors to read */
		cmd[9] = 0x00;		/* read no raw (main channel) data */
		cmd[10] = 0x01; 	/* Sub-Channel Selection: raw P-W=001*/
		/* cmd[11] = control byte */

		if (scsi_cmd(handle, cmd, sizeof cmd, data, data_len) != 0) {
			fprintf(stderr,
				"Warning: Cannot get ISRC code for track %d\n",
				track_num);
			return;
		}

		search_result = find_isrc_in_sector(data, isrc);
		if (search_result == ISRC_FOUND) {
			isrc_found = 1;
			break;
		} else if (search_result == CRC_MISMATCH) {
			fprintf(stderr, "Warning: CRC mismatch track %d: %s\n",
				track_num, isrc);
			warning_shown = 1;
		} /* otherwise keep searching the sectors */

		sector++;
		disc_offset++;
	}

	free(data);

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
