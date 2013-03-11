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

#include <stdio.h>
#include <string.h>

#include "discid/discid_private.h"
#include "scsi.h"

/* Send a scsi command and receive data. */
int scsi_cmd(int fd, unsigned char *cmd, int cmd_len,
	     unsigned char *data, int data_len) {
	return mb_scsi_cmd_unportable(fd, cmd, cmd_len, data, data_len);
}

static void decode_isrc(unsigned char *q_channel, char *isrc) {
	int isrc_pos;
	int data_pos;
	int bit_pos;
	int bit;
	unsigned char buffer;

	isrc_pos = 0;
	/* q_channel[0] = 0x03 -> mode 3 -> ISRC data */
	data_pos = 1;
	bit_pos = 7;
	buffer = 0;
	/* first 5 chars of ISRC are alphanumeric and 6-bit BCD encoded */
	for (bit = 0; bit < 5*6; bit++) {
		buffer = buffer << 1;
		if ((q_channel[data_pos] & (1 << bit_pos)) == (1 << bit_pos)) {
			buffer++;
		}
		bit_pos--;
		if ((bit + 1)%8 == 0) {
			bit_pos = 7;
			data_pos++;
		}
		if ((bit + 1)%6 == 0) {
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
	/* q_channel[9] are zero bits
	 * q_channel 10-12 are AFRAME
	 */

}

void mb_scsi_read_track_isrc(int fd, mb_disc_private *disc, int track_num) {
	int i;
	unsigned char cmd[10];
	unsigned char data[24];
	char buffer[ISRC_STR_LENGTH+1];

	memset(cmd, 0, sizeof cmd);
	memset(data, 0, sizeof data);
	memset(buffer, 0, sizeof buffer);

	cmd[0] = 0x42;		/* READ SUB-CHANNEL */
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
	/* there should be at least one ISRC in 100 sectors per spec
	 * We try 150 (= 2 seconds) to be sure, but break when successfull
	 */
	const int SEC_NUM = 150;
	unsigned char cmd[12];
	unsigned char data[SEC_NUM*2448]; /* overall data */
	unsigned char q_buffer;
	unsigned char q_data[12]; /* sub-channel data in 1 sector */
	char isrc[ISRC_STR_LENGTH+1];
	unsigned long offset;
	int char_num;
	int sector;
	int i;

	memset(cmd, 0, sizeof cmd);
	memset(data, 0, sizeof data);
	memset(isrc, 0, sizeof isrc);

	offset = disc->track_offsets[track_num];

	/* 0xbe = READ CD, implementation optional in contrast to 0x42
	 * support given with GET CONFIGURATION (0x46)
	 * Support part of:
	 * Multi-Read Feature (0x001d) and
	 * CD Read Feature (0x001e),
	 */
	cmd[0] = 0xbe;	/* READ CD */
	cmd[2] = offset >> 32;
	cmd[3] = offset >> 16;
	cmd[4] = offset >> 8;
	cmd[5] = offset;  /* from where to start reading */
	cmd[6] = SEC_NUM >> 16;
	cmd[7] = SEC_NUM >> 8;
	cmd[8] = SEC_NUM; /* sectors to read */
	cmd[9] = 0xF8; 	/* 11111000 sync=1 all-header=11 user=1, ecc=1
			 * no-error=00 */
	cmd[10] = 0x01; 	/* Sub-Channel Selection raw P-W=001*/
	/* cmd[11] = control byte */

	if (scsi_cmd(fd, cmd, sizeof cmd, data, sizeof data) != 0) {
		fprintf(stderr, "Warning: Cannot get ISRC code for track %d\n",
			track_num);
		return;
	}

	/* each sector has 96 bytes for one Q-channel type
	 * We try until we finde an ISRC Q-channel
	 */
	for (sector = 0; sector < SEC_NUM; sector++) {
		char_num = 0;
		memset(q_data, 0, sizeof q_data);
		q_buffer = 0;
		/* the first 2352 bytes are raw data without sub-channels
		 * We only want the 96 bit sub-channel data
		 */
		for (i = 2352; i < 2448; i++) {
			q_buffer = q_buffer << 1;
			/* the 6th bit is the Q-channel bit
			 * we want to collect these bits
			 */
			if ((data[i + (sector*2448)] & (1 << 6)) == (1 << 6)) {
				q_buffer++;
			}
			if ((i+1)%8 == 0) {
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
			/* we found a Q-channel with ISRC data */
			decode_isrc(q_data, isrc);
			break;
		}
	}
	strncpy(disc->isrc[track_num], isrc, ISRC_STR_LENGTH);
}


/* EOF */
