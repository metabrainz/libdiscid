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


/* Length of a MusicBrainz DiscID in bytes (without a trailing '\0'-byte). */
#define MB_DISC_ID_LENGTH	32

/* The maximum permitted length for an error message (without the '\0'-byte). */
#define MB_ERROR_MSG_LENGTH	255

/* URL prefix + 100 tracks, 5 bytes each + 32 bytes discid */
#define MB_MAX_URL_LENGTH	1023

/* TODO: "mm" necessary? Maybe even use a nicer URL? */
#define MB_SUBMISSION_URL	"http://mm.musicbrainz.org/bare/cdlookup.html"



/*
 * Transparent handle.
 */
/* typedef void *mb_disc; */


/*
 * This data structure represents an audio disc.
 *
 * We use fixed length strings here because that way the user doesn't have to
 * check for memory exhaustion conditions. As soon as the mb_disc object has
 * been created, all calls returning strings will be successful.
 */
typedef struct {
	int first_track_num;
	int last_track_num;
	int track_offsets[100];
	char id[MB_DISC_ID_LENGTH+1];
	char submission_url[MB_MAX_URL_LENGTH+1];
	char error_msg[MB_ERROR_MSG_LENGTH+1];
} mb_disc;


/*
 * Returns a pointer to a new mb_disc object.
 */
mb_disc *mb_disc_new();


/*
 * Release the memory allocated for the mb_disc object.
 */
void mb_disc_free(mb_disc *disc);


/*
 * Read the TOC from the given device and initialize the disc object.
 *
 * This function is unportable and has to be implemented once per operating
 * system.
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
 * TODO: access to first+last track, and offsets
 */

#endif /* MUSICBRAINZ_DISC_ID_H */
