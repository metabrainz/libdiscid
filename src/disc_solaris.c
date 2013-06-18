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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/cdio.h>


#include "discid/discid.h"
#include "discid/discid_private.h"
#include "unix.h"

#define NUM_CANDIDATES 2

static char *device_candidates[NUM_CANDIDATES] = {"/vol/dev/aliases/cdrom0",
					         "/volumes/dev/aliases/cdrom0"};

int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *toc) {
	struct cdrom_tochdr th;

	int ret = ioctl(fd, CDROMREADTOCHDR, &th);

	if ( ret < 0 )
		return ret; /* error */

	toc->first_track_num = th.cdth_trk0;
	toc->last_track_num = th.cdth_trk1;

	return ret;
}


int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *track) {
	struct cdrom_tocentry te;
	int ret;

	te.cdte_track = track_num;
	te.cdte_format = CDROM_LBA;

	ret = ioctl(fd, CDROMREADTOCENTRY, &te);
	assert( te.cdte_format == CDROM_LBA );

	if ( ret < 0 )
		return ret; /* error */

	track->address = te.cdte_addr.lba;
	track->control = te.cdte_ctrl;

	return ret;
}

void mb_disc_unix_read_mcn(int fd, mb_disc_private *disc) {
	return;
}

void mb_disc_unix_read_isrc(int fd, mb_disc_private *disc, int track_num) {
	return;
}

char *mb_disc_get_default_device_unportable(void) {
	int i;

	for (i = 0; i < NUM_CANDIDATES; i++) {
		if (mb_disc_unix_exists(device_candidates[i])) {
			fprintf(stderr, "%s\n",  device_candidates[i]);
			return device_candidates[i];
		}
	}
	/* use the first name for the error message later on */
	return device_candidates[0];
}

int mb_disc_has_feature_unportable(enum discid_feature feature) {
	switch(feature) {
		case DISCID_FEATURE_READ:
			return 1;
		default:
			return 0;
	}
}

/* EOF */
