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
#include "test.h"


int main(int argc, char *argv[]) {
	DiscId *d;
	DiscId *d2;
	int i, first, last;
	int subtest_passed;
	int offset, previous_offset;
	char *error_msg;
	int feature_read;
	int sectors;
	int *track_offsets;
	char *device;

	if (argc > 1) {
		device = argv[1];
	} else {
		device = NULL;
	}

	d = discid_new();

	announce("discid_has_feature");
	feature_read = discid_has_feature(DISCID_FEATURE_READ);
	evaluate(feature_read == 0 || feature_read == 1);

	announce("discid_read_sparse");
	if (!discid_read_sparse(d, device, 0)) {
		printf("SKIP\n");

		announce("discid_get_error_msg");
		error_msg = discid_get_error_msg(d);
		evaluate(strlen(error_msg) > 0);

		printf("\t%s\n\n", error_msg);
		discid_free(d);
		return 77; /* code for SKIP in autotools */
	}
	/* we shouldn't be here if the feature is not implemented */
	evaluate(discid_has_feature(DISCID_FEATURE_READ));

	announce("discid_get_default_device");
	/* In contrast to test_core, there should be a device now.  */
	evaluate(strlen(discid_get_default_device()) > 0);

	announce("discid_get_id");
	evaluate(equal_int(strlen(discid_get_id(d)), 28));

	announce("discid_get_freedb_id");
	evaluate(equal_int(strlen(discid_get_freedb_id(d)), 8));

	announce("discid_get_toc_string");
	evaluate(strlen(discid_get_toc_string(d)) > 0);

	announce("discid_get_submission_url");
	evaluate(strlen(discid_get_submission_url(d)) > 0);

	announce("discid_get_first_track_num");
	first = discid_get_first_track_num(d);
	evaluate(first > 0);
	announce("discid_get_last_track_num");
	last = discid_get_last_track_num(d);
	evaluate(last > 0);

	announce("discid_get_sectors");
	sectors = discid_get_sectors(d);
	evaluate(equal_int(sectors, discid_get_track_offset(d, last)
				+ discid_get_track_length(d, last)));

	announce("discid_get_track_offset sane");
	previous_offset = 0;
	subtest_passed = 0;
	for (i=first; i<=last; i++) {
		offset = discid_get_track_offset(d, i);
		if (offset <= sectors) {
			subtest_passed++;
		}
		if (previous_offset) {
			if (offset >= previous_offset) {
				subtest_passed++;
			}
		}
		previous_offset = offset;
	}
	evaluate(equal_int(subtest_passed, 2 * (last - first + 1) - 1));

	announce("discid_get_mcn empty");
	evaluate(strlen(discid_get_mcn(d)) == 0);

	announce("discid_get_track_isrc empty");
	subtest_passed = 0;
	for (i=first; i<=last; i++) {
		if (strlen(discid_get_track_isrc(d, i)) == 0) {
			subtest_passed++;
		}
	}
	evaluate(equal_int(subtest_passed, last - first + 1));

	announce("read/put idempotence");
	d2 = discid_new();
	/* create track offset array */
	track_offsets = malloc(sizeof (int) * (last - first + 2));
	memset(track_offsets, 0, sizeof (int) * (last - first + 2));
	track_offsets[0] = sectors;
	for (i=first; i<=last; i++) {
		track_offsets[i] = discid_get_track_offset(d, i);
	}
	discid_put(d2, first, last, track_offsets);
	evaluate(equal_str(discid_get_id(d2), discid_get_id(d))
			&& equal_str(discid_get_submission_url(d2),
				discid_get_submission_url(d)));
	free(track_offsets);
	discid_free(d2);

	announce("discid_get_error_msg");
	evaluate(strlen(discid_get_error_msg(d)) == 0);


	discid_free(d);

	return !test_result();
}

/* EOF */
