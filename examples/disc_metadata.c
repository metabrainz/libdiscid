/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <discid/discid.h>
#include <musicbrainz5/mb5_c.h>


static void print_medium(Mb5Medium medium)
{
	Mb5TrackList track_list;
	Mb5Track track;
	Mb5Recording recording;
	int required_string_length;
	char * buffer;
	int i;

	required_string_length = mb5_medium_get_title(medium, NULL, 0);
	buffer = (char *) malloc(required_string_length + 1);
	mb5_medium_get_title(medium, buffer, required_string_length + 1);

	printf("\tMedia  %d", mb5_medium_get_position(medium));
	if (strlen(buffer) > 0)
		printf("(%s)", buffer);
	printf(":\n");
	free(buffer);

	track_list = mb5_medium_get_tracklist(medium);

	for (i = 0; i < mb5_track_list_size(track_list); i++) {
		track = mb5_track_list_item(track_list, i);
		recording = mb5_track_get_recording(track);

		if (recording) {
			required_string_length = mb5_recording_get_title(
							recording, NULL, 0);
			buffer = (char *) malloc(required_string_length +1);
			mb5_recording_get_title(recording, buffer,
						required_string_length +1);
		} else {
			required_string_length = mb5_track_get_title(
							track, NULL, 0);
			buffer = (char *) malloc(required_string_length +1);
			mb5_track_get_title(track, buffer,
					    required_string_length +1);
		}

		printf("\t\tTrack %2d: %s\n",
		       mb5_track_get_position(track), buffer);
		free(buffer);
	}
}

static void print_release(Mb5Release release, char * disc_id)
{
	Mb5MediumList medium_list;
	Mb5Medium medium;
	int required_string_length;
	char * buffer;
	int must_delete_medium_list = 0;
	int i;

	/* Try to filter out the only the media we want */
	medium_list = mb5_release_media_matching_discid(release, disc_id);
	/* this function requires manual deletion */
	must_delete_medium_list = 1;
	if (mb5_medium_list_size(medium_list) == 0) {
		/* We probably received a fuzzy match
		 * and the DiscID is unknown to MB */
		medium_list = mb5_release_get_mediumlist(release);
		/* this is deleted together with the Release */
		must_delete_medium_list = 0;
		fprintf(stderr, "DiscID was not found, ");
		fprintf(stderr, "but we received a fuzzy match for the TOC.\n");
	}

	if (mb5_medium_list_size(medium_list) > 0) {
		required_string_length = mb5_release_get_title(release,
							       NULL, 0);
		buffer = (char *) malloc(required_string_length + 1);
		mb5_release_get_title(release, buffer,
				      required_string_length + 1);
		printf("\nRelease title: %s\n", buffer);
		free(buffer);

		for (i = 0; i < mb5_medium_list_size(medium_list); i++) {
			medium = mb5_medium_list_item(medium_list, i);
			print_medium(medium);
		}
	}

	if (must_delete_medium_list) {
		/* We must delete the result of 'media_matching_discid' */
		mb5_medium_list_delete(medium_list);
	}
}

static void handle_release_list(Mb5ReleaseList release_list, char * disc_id)
{
	Mb5Release Release;
	int i;

	for (i = 0; i < mb5_release_list_size(release_list); i++) {
		Release = mb5_release_list_item(release_list, i);
		print_release(Release, disc_id);
	}
}

int main(int argc, char *argv[])
{
	Mb5Query query;
	Mb5Metadata metadata;
	tQueryResult result_code;
	Mb5ReleaseList release_list;
	Mb5Disc result_disc;	/* libmusicbrainz disc result */
	DiscId drive_disc;	/* libdiscid disc object */
	char * disc_id = "";	/* tha actual disc ID */
	int http_code;
	char * drive = NULL;
	char error_msg[256];
	char * params[] = {"toc", "inc"};
	char * param_values[2] = {"", "recordings"};

	if (argc == 2)
		drive = (char *) argv[1];

	drive_disc = discid_new();
	if (!discid_read_sparse(drive_disc, drive, 0)) {
		fprintf(stderr, "Error: %s\n",
				discid_get_error_msg(drive_disc));
		discid_free(drive_disc);
		return 1;
	}
	disc_id = discid_get_id(drive_disc);
	param_values[0] = discid_get_toc_string(drive_disc);

	query = mb5_query_new("libdiscid_example-1.0", NULL, 0);
	metadata = mb5_query_query(query, "discid", disc_id, "",
				   2, params, param_values);
	/* we are done with the information from the drive */
	free(drive_disc);

	result_code = mb5_query_get_lastresult(query);
	http_code = mb5_query_get_lasthttpcode(query);
	mb5_query_get_lasterrormessage(query, error_msg, sizeof error_msg);
	if (strlen(error_msg) > 0) {
		fprintf(stderr, "ERROR: %s\n", error_msg);
		fprintf(stderr, "result: %d\thttp code: %d\n",
				result_code, http_code);
		return result_code;
	}

	if (metadata) {
		result_disc = mb5_metadata_get_disc(metadata);
		if (result_disc) {
			/* found by disc ID */
			release_list = mb5_disc_get_releaselist(result_disc);
		} else {
			/* disc ID not found, got a fuzzy match by TOC */
			release_list = mb5_metadata_get_releaselist(metadata);
		}
		handle_release_list(release_list, disc_id);

		mb5_metadata_delete(metadata);
	}
	mb5_query_delete(query);

	return 0;
}
