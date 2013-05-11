/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "discid/discid_private.h"
#include "unix.h"


/* TODO: make sure it's available */
int snprintf(char *str, size_t size, const char *format, ...);


int mb_disc_unix_read_toc(mb_disc_private *disc, mb_disc_toc *toc, const char *device) {
	int fd;
	int i;

	if ( (fd = open(device, O_RDONLY | O_NONBLOCK)) < 0 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"cannot open device `%s'", device);
		return 0;
	}

	/* Find the numbers of the first track (usually 1) and the last track. */
	if ( !mb_disc_unix_read_toc_header(fd, toc) ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"cannot read table of contents");
		close(fd);
		return 0;
	}

	/* basic error checking */
	if ( toc->last_track_num == 0 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"this disc has no tracks");
		close(fd);
		return 0;
	}

	/*
	 * Read the TOC entry for every track.
	 */
	for (i = toc->first_track_num; i <= toc->last_track_num; i++) {
		mb_disc_unix_read_toc_entry(fd, i, &toc->tracks[i]);
	}
	mb_disc_unix_read_toc_entry(fd, 0xAA, &toc->tracks[0]);

	close(fd);

	return 1;
}
/* EOF */
