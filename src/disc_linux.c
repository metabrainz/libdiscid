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

#define MB_DEFAULT_DEVICE "/dev/cdrom"
#define MAX_DEV_LEN 50

#if (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#	define THREAD_LOCAL __thread
#else
#	define THREAD_LOCAL
#endif

static THREAD_LOCAL char default_device[MAX_DEV_LEN] = "";
static THREAD_LOCAL int feature_warning_printed = 0;


static int get_device(int number, char *device, int device_len) {
	FILE *proc_file;
	char *current_device;
	char *lineptr = NULL;
	char *saveptr = NULL;
	size_t bufflen;
	int i, count, counter;
	int return_value = 0;

	proc_file = fopen("/proc/sys/dev/cdrom/info", "r");
	if (proc_file != NULL) {
		/* skip to line containing device names */
		do {
			if (getline(&lineptr, &bufflen, proc_file) < 0) {
				return 0;
			}
		} while (strstr(lineptr, "drive name:") == NULL);

		/* count number of devices = number of tabs - 1*/
		count = -1;
		for (i = 0; i < strlen(lineptr); i++) {
			if (lineptr[i] == '\t') count++;
		}

		/* go through devices, they are in reverse order */
		current_device = strtok_r(lineptr, "\t", &saveptr);
		/* skip column title */
		current_device = strtok_r(NULL, "\t", &saveptr);
		counter = count;
		while (current_device != NULL && counter >= number) {
			if (counter == number) {
				snprintf(device, device_len,
					 "/dev/%s", current_device);
				return_value = 1;
			}
			/* go to next in list */
			current_device = strtok_r(NULL, "\t", &saveptr);
			counter--;
		}

		/* trim the trailing \n for the last entry = first device */
		if (default_device[strlen(default_device)-1] == '\n') {
			default_device[strlen(default_device)-1] = '\0';
		}
		free(lineptr);
		fclose(proc_file);
	}
	return return_value;
}


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
	/* prefer the default device symlink to the internal names */
	if (mb_disc_unix_exists(MB_DEFAULT_DEVICE)) {
		return MB_DEFAULT_DEVICE;
	} else {
		if (get_device(1, default_device, MAX_DEV_LEN)) {
			return default_device;
		} else {
			return MB_DEFAULT_DEVICE;
		}
	}
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc) {
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
	mb_scsi_features features;
	memset(&handle, 0, sizeof handle);
	handle.fd = fd;
	features = mb_scsi_get_features(handle);
	if (features.raw_isrc) {
		mb_scsi_read_track_isrc_raw(handle, disc, track_num);
	} else {
		if (!feature_warning_printed) {
			fprintf(stderr, "Warning: raw ISRCs not available, using ISRCs given by subchannel read\n");
			feature_warning_printed = 1;
		}
		mb_scsi_read_track_isrc(handle, disc, track_num);
	}
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

int mb_disc_read_unportable(mb_disc_private *disc, const char *device,
			    unsigned int features) {
	char device_name[MAX_DEV_LEN] = "";
	int device_number;

	device_number = (int) strtol(device, NULL, 10);
	if (device_number > 0) {
		if(!get_device(device_number, device_name, MAX_DEV_LEN)) {
			snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
				 "cannot find cd device with the number '%d'",
				 device_number);
			return 0;
		} else {
			return mb_disc_unix_read(disc, device_name, features);
		}
	} else {
		return mb_disc_unix_read(disc, device, features);
	}
}

/* EOF */
