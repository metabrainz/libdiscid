/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender
   Copyright (C) 2006 Matthias Friedrich
   Copyright (C) 2000 Robert Kaye
   Copyright (C) 1999 Marc E E van Woerkom
   
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

     $Id$

--------------------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <scsi/sg.h>


#include "discid/discid.h"
#include "discid/discid_private.h"


#define MB_DEFAULT_DEVICE	"/dev/cdrom"

#define XA_INTERVAL		((60 + 90 + 2) * CD_FRAMES)

/* timeout better shouldn't happen for scsi commands -> device is reset */
#define DEFAULT_TIMEOUT 30000	/* in ms */

#ifndef SG_MAX_SENSE
#define SG_MAX_SENSE 16
#endif


static int read_toc_header(int fd, int *first, int *last) {
	struct cdrom_tochdr th;
	struct cdrom_multisession ms;

	int ret = ioctl(fd, CDROMREADTOCHDR, &th);

	if ( ret < 0 )
		return ret; /* error */

	*first = th.cdth_trk0;
	*last = th.cdth_trk1;

	/*
	 * Hide the last track if this is a multisession disc. Note that
	 * currently only dual-session discs with one track in the second
	 * session are handled correctly.
	 */
	ms.addr_format = CDROM_LBA;
	ret = ioctl(fd, CDROMMULTISESSION, &ms);

	if ( ms.xa_flag )
		(*last)--;

	return ret;
}


static int read_toc_entry(int fd, int track_num, unsigned long *lba) {
	struct cdrom_tocentry te;
	int ret;

	te.cdte_track = track_num;
	te.cdte_format = CDROM_LBA;

	ret = ioctl(fd, CDROMREADTOCENTRY, &te);
	assert( te.cdte_format == CDROM_LBA );

	/* in case the ioctl() was successful */
	if ( ret == 0 )
		*lba = te.cdte_addr.lba;

	return ret;
}


static int read_leadout(int fd, unsigned long *lba) {
	struct cdrom_multisession ms;
	int ret;

	ms.addr_format = CDROM_LBA;
	ret = ioctl(fd, CDROMMULTISESSION, &ms);

	if ( ms.xa_flag ) {
		*lba = ms.addr.lba - XA_INTERVAL;
		return ret;
	}

	return read_toc_entry(fd, CDROM_LEADOUT, lba);
}


char *mb_disc_get_default_device_unportable(void) {
	return MB_DEFAULT_DEVICE;
}

static void read_disc_mcn(int fd, mb_disc_private *disc)
{
	struct cdrom_mcn mcn;

	if(ioctl(fd, CDROM_GET_MCN, &mcn) == -1) {
		fprintf(stderr, "Warning: Unable to read the disc's media catalog number.\n");
	} else {
		strncpy( disc->mcn,
				(const char *)mcn.medium_catalog_number,
				MCN_STR_LENGTH );
	}
}

/* Send a scsi command and receive data. */
static int scsi_cmd(int fd, unsigned char *cmd, int cmd_len,
		    unsigned char *data, int data_len) {
	unsigned char sense_buffer[SG_MAX_SENSE]; /* for "error situations" */
	sg_io_hdr_t io_hdr;

	memset(&io_hdr, 0, sizeof io_hdr);

	assert(cmd_len <= 16);

	io_hdr.interface_id = 'S'; /* must always be 'S' (SCSI generic) */
	io_hdr.cmd_len = cmd_len;
	io_hdr.cmdp = cmd;
	io_hdr.timeout = DEFAULT_TIMEOUT; /* timeout in ms */
	io_hdr.sbp = sense_buffer;/* only used when status is CHECK_CONDITION */
	io_hdr.mx_sb_len = sizeof sense_buffer;
	io_hdr.flags = SG_FLAG_DIRECT_IO;

	io_hdr.dxferp = (void*)data;
	io_hdr.dxfer_len = data_len;
	io_hdr.dxfer_direction = SG_DXFER_FROM_DEV;

	if (ioctl(fd, SG_IO, &io_hdr) != 0) {
		return errno;
	} else {
		return io_hdr.status;	/* 0 = success */
	}
}

static void read_track_isrc(int fd, mb_disc_private *disc, int track_num) {
	int i;
	unsigned char cmd[10];
	unsigned char data[24];
	char buffer[ISRC_STR_LENGTH+1];

	memset(cmd, 0, sizeof cmd);
	memset(data, 0, sizeof data);
	memset(buffer, 0, sizeof buffer);

	/* data read from the last appropriate sector encountered
	 * by a current or previous media access operation.
	 * The Logical Unit accesses the media when there is/was no access.
	 * TODO: force access at a specific block? -> no duplicate ISRCs?
	 */
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

int mb_disc_has_feature_unportable(enum discid_feature feature) {
	switch(feature) {
		case DISCID_FEATURE_READ:
		case DISCID_FEATURE_MCN:
		case DISCID_FEATURE_ISRC:
			return 1;
		default:
			return 0;
	}
}


int mb_disc_read_unportable(mb_disc_private *disc, const char *device) {
	int fd;
	unsigned long lba;
	int first, last;
	int i;

	if ( (fd = open(device, O_RDONLY | O_NONBLOCK)) < 0 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"cannot open device `%s'", device);
		return 0;
	}

	/*
	 * Find the numbers of the first track (usually 1) and the last track.
	 */
	if ( read_toc_header(fd, &first, &last) < 0 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"cannot read table of contents");
		close(fd);
		return 0;
	}

	/* basic error checking */
	if ( last == 0 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"this disc has no tracks");
		close(fd);
		return 0;
	}

	/* Read in the media catalog number */
	read_disc_mcn( fd, disc );

	disc->first_track_num = first;
	disc->last_track_num = last;


	/*
	 * Get the logical block address (lba) for the end of the audio data.
	 * The "LEADOUT" track is the track beyond the final audio track, so
	 * we're looking for the block address of the LEADOUT track.
	 */
	read_leadout(fd, &lba);
	disc->track_offsets[0] = lba + 150;

	for (i = first; i <= last; i++) {
		read_toc_entry(fd, i, &lba);
		disc->track_offsets[i] = lba + 150;

		/* Read the ISRC for the track */
		read_track_isrc(fd, disc, i);
	}

	close(fd);

	return 1;
}

/* EOF */
