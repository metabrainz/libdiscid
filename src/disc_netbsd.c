/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2009 Anton Yabchinskiy
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

#include <sys/types.h>
#include <sys/cdio.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "discid/discid.h"
#include "discid/discid_private.h"
#include "unix.h"

#define NUM_CANDIDATES 2

/* rcd0c = OpenBSD (+ NetBSD?), rcd0d = x86 NetBSD */
static char *device_candidates[NUM_CANDIDATES] = {"/dev/rcd0c", "/dev/rcd0d"};


char *mb_disc_get_default_device_unportable(void) {
	return mb_disc_unix_find_device(device_candidates, NUM_CANDIDATES);
}


int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *toc) {
    struct ioc_toc_header th;

	int ret = ioctl(fd, CDIOREADTOCHEADER, &th);

	if ( ret < 0 )
		return 0; /* error */

	toc->first_track_num = th.starting_track;
	toc->last_track_num = th.ending_track;

	return 1;
}

int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *track) {
	struct cd_toc_entry te;
	struct ioc_read_toc_entry rte;
	int ret;

	memset(&rte, 0, sizeof rte);
	rte.address_format = CD_LBA_FORMAT;
	rte.data           = &te;
	rte.data_len       = sizeof te;
	rte.starting_track = track_num;

	ret = ioctl (fd, CDIOREADTOCENTRYS, &rte);

	if ( ret < 0 )
		return 0; /* error */

	track->address = te.addr.lba;
	track->control = te.control;

	return 1;
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc) {
	return;
}

void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc, int track_num) {
	return;
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
