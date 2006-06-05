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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA

     $Id$

--------------------------------------------------------------------------- */
#include <string.h>
#include <assert.h>

#include "sha1.h"
#include "base64.h"

#include "discid/discid.h"
#include "discid/discid_private.h"


#define TRACK_NUM_IS_VALID(disc, i) \
	( i >= disc->first_track_num && i <= disc->last_track_num )


static void mb_create_disc_id(mb_disc_private *disc, char buf[]);
static void mb_create_submission_url(mb_disc_private *disc, char buf[]);



/****************************************************************************
 *
 * Implementation of the public interface.
 *
 ****************************************************************************/

mb_disc *mb_disc_new() {
	/* initializes everything to zero */
	return calloc(1, sizeof(mb_disc_private));
}


void mb_disc_free(mb_disc *disc) {
	free(disc);
}


char *mb_disc_get_error_msg(mb_disc *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert( disc != NULL );

	return disc->error_msg;
}


char *mb_disc_get_id(mb_disc *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert( disc != NULL );
	assert( disc->success );

	if ( ! disc->success )
		return NULL;

	if ( strlen(disc->id) == 0 )
		mb_create_disc_id(disc, disc->id);

	return disc->id;
}


char *mb_disc_get_submission_url(mb_disc *d) {
	mb_disc_private *disc = (mb_disc_private *) d;
	assert( disc != NULL );
	assert( disc->success );

	if ( ! disc->success )
		return NULL;

	if ( strlen(disc->submission_url) == 0 )
		mb_create_submission_url(disc, disc->submission_url);

	return disc->submission_url;
}


int mb_disc_read(mb_disc *d, char *device) {
	mb_disc_private *disc = (mb_disc_private *) d;

	assert( disc != NULL );

	if ( device == NULL )
		device = mb_disc_get_default_device();

	assert( device != NULL );

	/* Necessary, because the disc handle could have been used before. */
	memset(disc, 0, sizeof(mb_disc_private));

	return disc->success = mb_disc_read_unportable(disc, device);
}


char *mb_disc_get_default_device(void) {
	return mb_disc_get_default_device_unportable();
}


int mb_disc_get_first_track_num(mb_disc *d) {
	mb_disc_private *disc = (mb_disc_private *) d;

	assert( disc != NULL );

	return disc->first_track_num;
}


int mb_disc_get_last_track_num(mb_disc *d) {
	mb_disc_private *disc = (mb_disc_private *) d;

	assert( disc != NULL );

	return disc->last_track_num;
}


int mb_disc_get_sectors(mb_disc *d) {
	mb_disc_private *disc = (mb_disc_private *) d;

	assert( disc != NULL );

	return disc->track_offsets[0];
}


int mb_disc_get_track_offset(mb_disc *d, int i) {
	mb_disc_private *disc = (mb_disc_private *) d;

	assert( disc != NULL );
	assert( TRACK_NUM_IS_VALID(disc, i) );

	if ( ! TRACK_NUM_IS_VALID(disc, i) )
		return 0;

	return disc->track_offsets[i];
}


int mb_disc_get_track_length(mb_disc *d, int i) {
	mb_disc_private *disc = (mb_disc_private *) d;

	assert( disc != NULL );
	assert( TRACK_NUM_IS_VALID(disc, i) );

	if ( ! TRACK_NUM_IS_VALID(disc, i) )
		return 0;

	if ( i < disc->last_track_num )
		return disc->track_offsets[i+1] - disc->track_offsets[i];
	else
		return disc->track_offsets[0] - disc->track_offsets[i];
}


/****************************************************************************
 *
 * Private utilities, not exported.
 *
 ****************************************************************************/

/*
 * Create a DiscID based on the TOC data found in the mb_disc object.
 * The DiscID is placed in the provided string buffer.
 */
static void mb_create_disc_id(mb_disc_private *disc, char buf[]) {
	SHA_INFO	sha;
	unsigned char	digest[20], *base64;
	unsigned long	size;
	char		tmp[17]; /* for 8 hex digits (16 to avoid trouble) */
	int		i;

	assert( disc != NULL );

	sha_init(&sha);

	sprintf(tmp, "%02X", disc->first_track_num);
	sha_update(&sha, (unsigned char *) tmp, strlen(tmp));

	sprintf(tmp, "%02X", disc->last_track_num);
	sha_update(&sha, (unsigned char *) tmp, strlen(tmp));

	for (i = 0; i < 100; i++) {
		sprintf(tmp, "%08X", disc->track_offsets[i]);
		sha_update(&sha, (unsigned char *) tmp, strlen(tmp));
	}

	sha_final(digest, &sha);

	base64 = rfc822_binary(digest, sizeof(digest), &size);

	memcpy(buf, base64, size);
	buf[size] = '\0';

	free(base64);
}


/*
 * Create a submission URL based on the TOC data found in the mb_disc object.
 * The URL is placed in the provided string buffer.
 */
static void mb_create_submission_url(mb_disc_private *disc, char buf[]) {
	char tmp[1024];
	int i;

	assert( disc != NULL );

	strcpy(buf, MB_SUBMISSION_URL);

	strcat(buf, "?id=");
	strcat(buf, mb_disc_get_id((mb_disc *) disc));

	sprintf(tmp, "&tracks=%d", disc->last_track_num);
	strcat(buf, tmp);

	sprintf(tmp, "&toc=%d+%d+%d",
			disc->first_track_num,
			disc->last_track_num,
			disc->track_offsets[0]);
	strcat(buf, tmp);

	for (i = disc->first_track_num; i <= disc->last_track_num; i++) {
		sprintf(tmp, "+%d", disc->track_offsets[i]);
		strcat(buf, tmp);
	}
}

/* EOF */
