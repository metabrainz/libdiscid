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


int main(int argc, char *argv[]) {
	DiscId *d;
	int i, first, last;
	int found, invalid;
	char *mcn;
	char *isrc;
	char *error_msg;
	int feature_mcn, feature_isrc;
	char *device;

	if (argc > 1) {
		device = argv[1];
	} else {
		device = NULL;
	}

	d = discid_new();

	announce("discid_has_feature");
        feature_mcn = discid_has_feature(DISCID_FEATURE_MCN);
        feature_isrc = discid_has_feature(DISCID_FEATURE_ISRC);
        evaluate((feature_mcn == 0 || feature_mcn == 1)
			&& (feature_isrc == 0 || feature_isrc == 1));

	announce("discid_read");
	if (!discid_read(d, device)) {
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


	announce("discid_get_id");
	evaluate(equal_int(strlen(discid_get_id(d)), 28));

	announce("discid_get_toc_string");
	evaluate(strlen(discid_get_toc_string(d)) > 0);

	announce("discid_get_submission_url");
	evaluate(strlen(discid_get_submission_url(d)) > 0);

	first = discid_get_first_track_num(d);
	last = discid_get_last_track_num(d);

	/* Even if everything works, there might not be an MCN on the disc */
	announce("discid_get_mcn");
	mcn = discid_get_mcn(d);
	if (discid_has_feature(DISCID_FEATURE_MCN)) {
		evaluate(strlen(mcn) == 0 || strlen(mcn) == 13);
	} else {
		evaluate(strlen(mcn) == 0);
	}

	/* Even if everything works, there might not be ISRCs for all tracks */
	announce("discid_get_track_isrc");
	found = 0;
	invalid = 0;
	for (i=first; i<=last; i++) {
		isrc = discid_get_track_isrc(d, i);
		if (strlen(isrc) == 12) {
			found++;
		} else if (strlen(isrc) != 0) {
			invalid++;
			break;
		}
	}
	if (discid_has_feature(DISCID_FEATURE_ISRC)) {
		evaluate(!invalid);
	} else {
		evaluate(!invalid && !found);
	}

	announce("discid_get_error_msg");
        evaluate(strlen(discid_get_error_msg(d)) == 0);


	discid_free(d);

	return !test_result();
}

/* EOF */
