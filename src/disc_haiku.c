/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2009 Shunsuke Kuroda
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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301  USA

--------------------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <scsi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


#include "discid/discid.h"
#include "discid/discid_private.h"
#include "unix.h"

#define NUM_CANDIDATES 4

static char *device_candidates[NUM_CANDIDATES] = {
	"/dev/disk/atapi/0/master/raw",
	"/dev/disk/atapi/0/slave/raw",
	"/dev/disk/atapi/1/master/raw",
	"/dev/disk/atapi/1/slave/raw"
};

int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *disk_toc) {
	scsi_toc toc;
	int ret = ioctl(fd, B_SCSI_GET_TOC, &toc);

	if (ret == -1)
		return 0; /* error */

	disk_toc->first_track_num = toc.toc_data[2];
	disk_toc->last_track_num = toc.toc_data[3];

	return 1;
}


int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *track) {
	scsi_toc toc;
	int ret, base, address;

	ret = ioctl(fd, B_SCSI_GET_TOC, &toc);

	if (ret == -1)
		return 0; /* error */

	/* leadout - track number 170 (0xaa) */
	if (track_num == 0xaa)
		track_num = toc.toc_data[3] + 1;

	base = ((track_num - toc.toc_data[2]) * 8) + 4;

	/* out of bound */
	if (base >= sizeof(toc.toc_data) / sizeof(toc.toc_data[0]) ||
		base + 7 >= sizeof(toc.toc_data) / sizeof(toc.toc_data[0]) ||
		base < 0)
		return 0;

	/* LBA = (minutes * 60 + seconds) * 75 + frames - 150 */
	address = toc.toc_data[base + 5];
	address *= 60;
	address += toc.toc_data[base + 6];
	address *= 75;
	address += toc.toc_data[base + 7];
	address -= 150;
	track->address = address;

	track->control = toc.toc_data[base + 1];

	return 1;
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc) {
	return;
}

void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc, int track_num) {
	return;
}

char *mb_disc_get_default_device_unportable(void) {
	return mb_disc_unix_find_device(device_candidates, NUM_CANDIDATES);
}

int mb_disc_has_feature_unportable(enum discid_feature feature) {
	switch(feature) {
		case DISCID_FEATURE_READ:
			return 1;
		default:
			return 0;
	}
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device,
			    unsigned int features) {
	return mb_disc_unix_read(disc, device, features);
}

/* EOF */
