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


/**
 * A transparent handle for an Audio CD.
 *
 * This is returned by mb_disc_new() and has to be passed as the first
 * parameter to all mb_disc_*() functions.
 */
typedef void *mb_disc;


/**
 * Return a handle for a new mb_disc object.
 *
 * If no memory could be allocated, NULL is returned. Don't use the created
 * mb_disc object before calling mb_disc_read().
 *
 * @return an mb_disc object, or NULL.
 */
mb_disc *mb_disc_new();


/**
 * Release the memory allocated for the mb_disc object.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 */
void mb_disc_free(mb_disc *disc);


/**
 * Read the disc in the given CD-ROM/DVD-ROM drive.
 *
 * This function reads the disc in the drive specified by the given device
 * identifier. If the device is NULL, the default drive, as returned by
 * mb_disc_get_default_device() is used.
 *
 * On error, this function returns false and sets the error message which you
 * can access using mb_disc_get_error_msg(). In this case, the other functions
 * won't return meaningful values and should not be used.
 *
 * This function may be used multiple times with the same mb_disc object.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @param device an operating system dependent device identifier, or NULL
 * @return true if successful, and false on error.
 */
int mb_disc_read(mb_disc *disc, char *device);


/**
 * Return a human-readable error message.
 *
 * This function may only be used if mb_disc_read() failed. The returned
 * error message is only valid as long as the mb_disc object exists.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @return a string describing the error that occurred
 */
char *mb_disc_get_error_msg(mb_disc *disc);


/**
 * Return a MusicBrainz DiscID.
 *
 * The returned string is only valid as long as the mb_disc object exists.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @return a string containing a MusicBrainz DiscID
 */
char *mb_disc_get_id(mb_disc *disc);


/**
 * Return an URL for submitting the DiscID to MusicBrainz.
 *
 * The URL leads to an interactive disc submission wizard that guides the
 * user through the process of associating this disc's DiscID with a
 * release in the MusicBrainz database.
 *
 * The returned string is only valid as long as the mb_disc object exists.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @return a string containing an URL
 */
char *mb_disc_get_submission_url(mb_disc *disc);


/**
 * Return the name of the default disc drive for this operating system.
 *
 * @param a string containing an operating system dependent device identifier
 */
char *mb_disc_get_default_device(void);


/**
 * Return the number of the first track on this disc.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @return the number of the first track
 */
int mb_disc_get_first_track_num(mb_disc *disc);


/**
 * Return the number of the last track on this disc.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @return the number of the last track
 */
int mb_disc_get_last_track_num(mb_disc *disc);


/**
 * Return the length of the disc in sectors.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @return the length of the disc in sectors
 */
int mb_disc_get_sectors(mb_disc *disc);


/**
 * Return the sector offset of a track.
 *
 * Only track numbers between (and including) mb_disc_get_first_track_num()
 * and mb_disc_get_last_track_num() may be used.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @param track_num the number of a track
 * @return a pointer to an array of track offsets
 */
int mb_disc_get_track_offset(mb_disc *d, int track_num);


/**
 * Return the length of a track in sectors.
 *
 * Only track numbers between (and including) mb_disc_get_first_track_num()
 * and mb_disc_get_last_track_num() may be used.
 *
 * @param disc an mb_disc object created by mb_disc_new()
 * @param track_num the number of a track
 * @return a pointer to an array of track offsets
 */
int mb_disc_get_track_length(mb_disc *d, int track_num);


#endif /* MUSICBRAINZ_DISC_ID_H */
