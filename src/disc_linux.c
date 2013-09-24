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
#include <scsi/scsi.h>
#include <scsi/sg.h>


#include "discid/discid.h"
#include "discid/discid_private.h"
#include "scsi.h"
#include "unix.h"


#define MB_DEFAULT_DEVICE	"/dev/cdrom"


/* timeout better shouldn't happen for scsi commands -> device is reset */
#define DEFAULT_TIMEOUT 30000	/* in ms */

#ifndef SG_MAX_SENSE
#define SG_MAX_SENSE 16
#endif


int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *toc) {
	struct cdrom_tochdr th;

	int ret = ioctl(fd, CDROMREADTOCHDR, &th);

	if ( ret < 0 )
		return 0; /* error */

	toc->first_track_num = th.cdth_trk0;
	toc->last_track_num = th.cdth_trk1;

	return 1;
}


int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *track) {
	struct cdrom_tocentry te;
	int ret;

	te.cdte_track = track_num;
	te.cdte_format = CDROM_LBA;

	ret = ioctl(fd, CDROMREADTOCENTRY, &te);
	assert( te.cdte_format == CDROM_LBA );

	if ( ret < 0 )
		return 0; /* error */

	track->address = te.cdte_addr.lba;
	track->control = te.cdte_ctrl;

	return 1;
}

char *mb_disc_get_default_device_unportable(void) {
	return MB_DEFAULT_DEVICE;
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc)
{
	struct cdrom_mcn mcn;
	memset(&mcn, 0, sizeof mcn);

	if(ioctl(fd, CDROM_GET_MCN, &mcn) == -1) {
		fprintf(stderr, "Warning: Unable to read the disc's media catalog number.\n");
	} else {
		strncpy( disc->mcn,
				(const char *)mcn.medium_catalog_number,
				MCN_STR_LENGTH );
	}
}

/* Send a scsi command and receive data. */
mb_scsi_status mb_scsi_cmd_unportable(mb_scsi_handle handle,
			unsigned char *cmd, int cmd_len,
			unsigned char *data, int data_len) {
	unsigned char sense_buffer[SG_MAX_SENSE]; /* for "error situations" */
	sg_io_hdr_t io_hdr;
	int return_value;

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

	return_value = ioctl(handle.fd, SG_IO, &io_hdr);

	if (return_value == -1) {
		/* failure */
		fprintf(stderr, "ioctl error: %d\n", errno);
		return IO_ERROR;
	} else {
		/* check for potenticall informative success codes
		 * 1 seems to be what is mostly returned */
		if (return_value != 0) {
			fprintf(stderr, "ioctl return value: %d\n",
					return_value);
			/* no error, but possibly informative */
		}

		/* check scsi status */
		if (io_hdr.masked_status != GOOD) {
			fprintf(stderr, "scsi status: %d\n", io_hdr.status);
			return STATUS_ERROR;
		} else if (data_len > 0 && io_hdr.resid == data_len) {
			/* not receiving data, when requested */
			fprintf(stderr, "data requested, but none returned\n");
			return NO_DATA_RETURNED;
		} else {
			return SUCCESS;
		}
	}
}

void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc, int track_num) {
	mb_scsi_handle handle;
	memset(&handle, 0, sizeof handle);
	handle.fd = fd;
	// TODO: test if raw actually is available
	//mb_scsi_read_track_isrc(handle, disc, track_numi);
	mb_scsi_read_track_isrc_raw(handle, disc, track_num);
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

/* EOF */
