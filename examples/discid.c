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
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------------- */
#include <stdio.h>
#include <discid/discid.h>

#ifndef DISCID_HAVE_SPARSE_READ
#define discid_read_sparse(disc, dev, i) discid_read(disc, dev)
#endif

#define FRAMES_PER_SECOND 75.0

int main(int argc, char *argv[]) {
	DiscId *disc = discid_new();
	int i, first_track, last_track;
	char *device = NULL;

	/* If we have an argument, use it as the device name */
	if (argc > 1) {
		device = argv[1];
	}

	/* read the disc in the specified (or default) disc drive */
	if (discid_read_sparse(disc, device, 0) == 0) {
		fprintf(stderr, "Error: %s\n", discid_get_error_msg(disc));
		return 1;
	}

	printf("DiscID        : %s\n", discid_get_id(disc));
	printf("FreeDB DiscID : %s\n", discid_get_freedb_id(disc));

	first_track = discid_get_first_track_num(disc);
	last_track = discid_get_last_track_num(disc);
	printf("First track   : %d\n", first_track);
	printf("Last track    : %d\n", last_track);

	printf("Length        : %d frames\n", discid_get_sectors(disc));

	for ( i = first_track; i <= last_track; i++ ) {
		int frames = discid_get_track_length(disc, i);
		float duration_in_secs = (float) frames / FRAMES_PER_SECOND;
		int minutes = duration_in_secs / 60;
		float seconds = duration_in_secs - minutes * 60;

		printf("Track %-2d      : %8d %8d (%02d:%05.2f)\n",
				i, discid_get_track_offset(disc, i),
				frames, minutes, seconds);
	}

	printf("Submit via    : %s\n", discid_get_submission_url(disc));

	discid_free(disc);

	return 0;
}

/* EOF */
