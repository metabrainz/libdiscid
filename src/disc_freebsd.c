/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2008 Patrick Hurrelmann 
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
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/cdio.h>
#include <arpa/inet.h>


#include "discid/discid.h"
#include "discid/discid_private.h"

#define MB_DEFAULT_DEVICE		"/dev/acd0"


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
	struct ioc_read_toc_single_entry te;
	int ret;

	te.track = track_num;
	te.address_format = CD_LBA_FORMAT;

	ret = ioctl(fd, CDIOREADTOCENTRY, &te);
	assert( te.address_format == CD_LBA_FORMAT );

	if ( ret < 0 )
		return 0; /* error */

	track->address = ntohl(te.entry.addr.lba);
	track->control = te.entry.control;

	return 1;
}


char *mb_disc_get_default_device_unportable(void) {
	return MB_DEFAULT_DEVICE;
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
	mb_disc_toc toc;

	if ( !mb_disc_unix_read_toc(disc, &toc, device) )
		return 0;

	if ( !mb_disc_load_toc(disc, &toc) )
		return 0;

	return 1;
}

/* EOF */
