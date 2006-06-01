#include <string.h>
#include <assert.h>

#include "sha1.h"
#include "base64.h"

#include "discid.h"


static void mb_create_disc_id(mb_disc *disc, char buf[]);
static void mb_create_submission_url(mb_disc *disc, char buf[]);



/****************************************************************************
 *
 * Implementation of the public interface.
 *
 ****************************************************************************/


mb_disc *mb_disc_new() {
	return calloc(1, sizeof(mb_disc)); /* initializes everything to zero */
}


void mb_disc_free(mb_disc *disc) {
	free(disc);
}


char *mb_disc_get_error_msg(mb_disc *disc) {
	assert( disc != NULL );

	return disc->error_msg; /* TODO */
}


char *mb_disc_get_id(mb_disc *disc) {
	assert( disc != NULL );

	if ( strlen(disc->id) == 0 )
		mb_create_disc_id(disc, disc->id);

	return disc->id;
}


char *mb_disc_get_submission_url(mb_disc *disc) {
	assert( disc != NULL );

	if ( strlen(disc->submission_url) == 0 )
		mb_create_submission_url(disc, disc->submission_url);

	return disc->submission_url;
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
static void mb_create_disc_id(mb_disc *disc, char buf[]) {
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
static void mb_create_submission_url(mb_disc *disc, char buf[]) {
	char tmp[1024];
	int i;

	assert( disc != NULL );

	strcpy(buf, MB_SUBMISSION_URL);

	strcat(buf, "?id=");
	strcat(buf, mb_disc_get_id(disc));

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
