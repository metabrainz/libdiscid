#include <stdio.h>
#include <discid.h>

int main(void) {
	mb_disc *disc = mb_disc_new();

	/* read the disc in the default disc drive */
	if ( mb_disc_read(disc, NULL) == 0 ) {
		fprintf(stderr, "Error: %s\n", mb_disc_get_error_msg(disc));
		return 1;
	}

	printf("      DiscID: %s\n", mb_disc_get_id(disc));

	printf(" First track: %d\n", mb_disc_get_first_track_num(disc));
	printf("  Last track: %d\n", mb_disc_get_last_track_num(disc));

	printf(" Disc Length: %d sectors\n", mb_disc_get_sectors(disc));

	printf("  Submit via: %s\n", mb_disc_get_submission_url(disc));

	mb_disc_free(disc);

	return 0;
}
