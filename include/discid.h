/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2006 Matthias Friedrich
   
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
#ifndef MUSICBRAINZ_DISC_ID_H
#define MUSICBRAINZ_DISC_ID_H


/*
 * A transparent handle.
 */
typedef void *mb_disc;


/*
 * Returns a pointer to a new mb_disc object.
 */
mb_disc *mb_disc_new();


/*
 * Release the memory allocated for the mb_disc object.
 */
void mb_disc_free(mb_disc *disc);


/*
 * Read the TOC from the given device and initialize the mb_disc object.
 *
 * Returns 0 on error.
 */
int mb_disc_read(mb_disc *disc, char *device);


/*
 * Returns an error message. Only valid if mb_disc_read() returned false.
 * The returned string is only valid as long as the mb_disc exists.
 */
char *mb_disc_get_error_msg(mb_disc *disc);


/*
 * Returns a MusicBrainz DiscID.
 *
 * The returned string is only valid as long as the mb_disc exists.
 */
char *mb_disc_get_id(mb_disc *disc);


/*
 * The returned string is only valid as long as the mb_disc exists.
 */
char *mb_disc_get_submission_url(mb_disc *disc);


/*
 * Returns the name of the default disc device for this operating system.
 */
char *mb_disc_get_default_device(void);


/*
 * Returns the number of the first track on this disc.
 */
int mb_disc_get_first_track_num(mb_disc *disc);


/*
 * Returns the number of the last track on this disc.
 */
int mb_disc_get_last_track_num(mb_disc *disc);


/*
 * Returns the length of the disc in sectors.
 */
int mb_disc_get_sectors(mb_disc *disc);


/*
 * TODO: how should the interface for mb_disc_get_tracks() look like?
 * int *mb_disc_get_tracks(mb_disc *disc), returning the track offsets?
 */

#endif /* MUSICBRAINZ_DISC_ID_H */
