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

     $Id$

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

#define		CD_FRAMES		75 /* per second */
#define		CD_DATA_TRACK		0x04
#define		CD_LEADOUT		0xAA

#define MB_DEFAULT_DEVICE		"/dev/acd0"

#define XA_INTERVAL			((60 + 90 + 2) * CD_FRAMES)


static int read_toc_header(int fd, int *first, int *last) {
	struct ioc_toc_header th;
	struct ioc_read_toc_single_entry te;

	int ret = ioctl(fd, CDIOREADTOCHEADER, &th);

	if ( ret < 0 )
		return ret; /* error */

	*first = th.starting_track;
	*last = th.ending_track;

	/*
	 * Hide the last track if this is a multisession disc. Note that
	 * currently only dual-session discs with one track in the second
	 * session are handled correctly.
	 */
	te.address_format = CD_LBA_FORMAT;
	te.track = th.ending_track;
	ret = ioctl(fd, CDIOREADTOCENTRY, &te);

	if (( te.entry.control & CD_DATA_TRACK) != 0 )
		(*last)--;

	return ret;
}


static int read_toc_entry(int fd, int track_num, unsigned long *lba) {
	struct ioc_read_toc_single_entry te;
	int ret;

	te.track = track_num;
	te.address_format = CD_LBA_FORMAT;

	ret = ioctl(fd, CDIOREADTOCENTRY, &te);
	assert( te.address_format == CD_LBA_FORMAT );

	if ( ret == 0 )
		*lba = ntohl(te.entry.addr.lba);

	return ret;
}


static int read_leadout(int fd, unsigned long *lba) {
	struct ioc_toc_header th;
	struct ioc_read_toc_single_entry te;
	int ret;
   
	ret = ioctl(fd, CDIOREADTOCHEADER, &th);
	te.track = th.ending_track;
	te.address_format = CD_LBA_FORMAT;
	ret = ioctl(fd, CDIOREADTOCENTRY, &te);

	if (( te.entry.control & CD_DATA_TRACK) != 0 ) {
		*lba = ntohl(te.entry.addr.lba) - 11400;
		return ret;
	}

	return read_toc_entry(fd, CD_LEADOUT, lba);
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

	/*
	 * Now, for every track, find out the block address where it starts.
	 */
	for (i = first; i <= last; i++) {
		read_toc_entry(fd, i, &lba);
		disc->track_offsets[i] = lba + 150;
	}

	close(fd);

	return 1;
}

/* EOF */
