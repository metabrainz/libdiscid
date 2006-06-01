#include <stdio.h>
#include <discid.h>

int main(void) {
	mb_disc *disc = mb_disc_new();

	if ( mb_disc_read(disc, "/dev/cdrom") == 0 ) {
		fprintf(stderr, "Error: %s\n", mb_disc_get_error_msg(disc));
		return 1;
	}

	printf("DiscID: %s\n", mb_disc_get_id(disc));

	printf("Submit via: %s\n", mb_disc_get_submission_url(disc));

	mb_disc_free(disc);

	return 0;
}
