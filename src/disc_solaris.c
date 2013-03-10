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

     $Id$

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


#define MB_DEFAULT_DEVICE	"/vol/dev/aliases/cdrom0"

#define CD_FRAMES		75
#define XA_INTERVAL		((60 + 90 + 2) * CD_FRAMES)


static int read_toc_header(int fd, int *first, int *last) {
	int ret;
	unsigned long ms_offset;

	struct cdrom_tochdr th;
	struct cdrom_tocentry te;

	ret = ioctl(fd, CDROMREADTOCHDR, &th);

	if ( ret < 0 )
		return ret; /* error */

	*first = th.cdth_trk0;
	*last  = th.cdth_trk1;

	/*
	 * Hide the last track if this is a multisession disc.
	 *
	 * te.cdte_addr.lba is the absolute CD-ROM address of the first
	 * track in the FIRST SESSION. ms_offset is the absolute CD-ROM
	 * address of the first track in the LAST SESSION.
	 *
	 * If ms_offset differs from te.cdte_addr.lba, CD-ROM is multi-
	 * session.
	 *
	 * Note that currently only dual-session discs with one track
	 * in the second session are handled correctly.
	 */
	te.cdte_track  = 1;
	te.cdte_format = CDROM_LBA;

	ret = ioctl(fd, CDROMREADTOCENTRY, &te);
	ret = ioctl(fd, CDROMREADOFFSET,   &ms_offset);

	if ( ms_offset != te.cdte_addr.lba  )
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
	int ret;
	unsigned long ms_offset;

	struct cdrom_tocentry te;

	te.cdte_track  = 1;
	te.cdte_format = CDROM_LBA;

	ret = ioctl(fd, CDROMREADTOCENTRY, &te);
	ret = ioctl(fd, CDROMREADOFFSET,   &ms_offset);

	if ( ms_offset != te.cdte_addr.lba  ) {
		*lba = ms_offset - XA_INTERVAL;
		return ret;
	}

	return read_toc_entry(fd, CDROM_LEADOUT, lba);
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


int mb_disc_read_unportable(mb_disc_private *disc, const char *device, unsigned int features) {
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
	}

	close(fd);

	return 1;
}

/* EOF */
