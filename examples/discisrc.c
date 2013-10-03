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


int main(int argc, char *argv[]) {
	DiscId *disc;
	int i;
	char *device;
	char *features[DISCID_FEATURE_LENGTH];

	printf("%s\n", discid_get_version_string());

	/* If we have an argument, use it as the device name */
	if (argc > 1) {
		device = argv[1];
	} else {
		device = discid_get_default_device();
	}

	printf("Device used: %s\n", device);

	if (!discid_has_feature(DISCID_FEATURE_READ)) {
		fprintf(stderr, "Error: not implemented on platform\n");
		return 1;
	} else {
		disc = discid_new();
	}


	/* read the disc in the specified disc drive with the MCN and ISRC feature enabled */
	if (discid_read_sparse(disc, device, DISCID_FEATURE_MCN | DISCID_FEATURE_ISRC) == 0) {
		fprintf(stderr, "Error: %s\n", discid_get_error_msg(disc));
		discid_free(disc);
		return 1;
	}

	if (discid_has_feature(DISCID_FEATURE_MCN)) {
		printf("MCN        : %s\n", discid_get_mcn(disc));
	} else {
		printf("MCN        : (not implemented)\n");
	}

	if (discid_has_feature(DISCID_FEATURE_ISRC)) {
		for ( i = discid_get_first_track_num(disc);
				i <= discid_get_last_track_num(disc); i++ ) {

			printf("Track %-2d   : %s\n", i,
					discid_get_track_isrc(disc, i));
		}
	} else {
		printf("ISRCs      : (not implemented)\n");
	}

	/* another way to access the features */
	discid_get_feature_list(features);
	printf("All features: ");
	for (i = 0; i < DISCID_FEATURE_LENGTH; i++) {
		if (features[i]) {
			if (i > 0) {
				printf(", ");
			}
			printf("%s", features[i]);
		}
	}
	printf("\n");

	discid_free(disc);

	return 0;
}

/* EOF */
