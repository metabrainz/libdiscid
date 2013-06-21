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
#include <string.h>

#include <discid/discid.h>
#include "test.h"

static int test_read_simple() {
	DiscId *d;
	int i, first, last;
	int offset, previous_offset;

	d = discid_new();
	discid_read_sparse(d, NULL, 0);

	if (!equal_int(strlen(discid_get_id(d)), 28, "Invalid Disc ID")) {
		discid_free(d); return 0;
	}
	if (!equal_int(strlen(discid_get_freedb_id(d)), 8,
				"Invalid FreeDB ID")) {
		discid_free(d); return 0;
	}
	if (!assert_true(strlen(discid_get_submission_url(d)) > 0,
				"No submission url")) {
		discid_free(d); return 0;
	}
	first = discid_get_first_track_num(d);
	last = discid_get_last_track_num(d);
	if (!equal_int(discid_get_sectors(d),
				discid_get_track_offset(d, last)
				+ discid_get_track_length(d, last),
				"Disc length mismatch")) {
		discid_free(d); return 0;
	}
	previous_offset = 0;
	for (i=first; i<=last; i++) {
		offset = discid_get_track_offset(d, i);
		if (!assert_true(offset <= discid_get_sectors(d),
					"Invalid offset")) {
			discid_free(d); return 0;
		}
		if (previous_offset) {
			if (!assert_true(offset >= previous_offset,
						"Invalid offset series")) {
				discid_free(d); return 0;
			}
		}
		previous_offset = offset;
	}

	/* additional features unset */
	if (!assert_true(strlen(discid_get_mcn(d)) == 0, "MCN not unset")) {
		discid_free(d); return 0;
	}
	for (i=first; i<=last; i++) {
		if (!assert_true(strlen(discid_get_track_isrc(d, i)) == 0,
					"ISRC not unset")) {
			discid_free(d); return 0;
		}
	}

	discid_free(d);
	return 1;
}

static int check_for_disc() {
	DiscId *d;
	int result;
	d = discid_new();
	result = discid_read_sparse(d, NULL, 0);
	discid_free(d);
	return result;
}

int main(int argc, char *argv[]) {

	if (!check_for_disc()) {
		printf("No disc found!\n");
		printf("SKIPPING!\n\n");
		return 77; /* code for SKIP in autotools */
	}

	announce("simple read");
	evaluate(test_read_simple());

	return !test_result();
}

/* EOF */
