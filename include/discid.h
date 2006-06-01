/*
 * discid.h - Proposed interface for libdiscid
 *
 * libdiscid will become a C library for generating MusicBrainz DiscIDs
 * on as many different operating systems as possible. It can be shipped
 * with libmb3 or separately.
 *
 * The idea is to provide a library which is easy to use from script languages
 * via python's ctypes etc. but is more lightweight than libmb3 and has no
 * further dependencies.
 */
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
 *
 * TODO: Is a char pointer OK? libmb2 uses a custom type.
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
 * TODO: access to first+last track, and offsets
 */

#endif /* MUSICBRAINZ_DISC_ID_H */
