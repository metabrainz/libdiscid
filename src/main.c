#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#include "disc.h"

int main(int argc, char *argv[]) {
	struct mb_disc *disc;
	char buf[33];
	char *url;

	if ( (disc = mb_read_disc("/dev/cdrom")) == NULL ) {
		fprintf(stderr, "Horror\n");
		perror("Error");
		return 0;
	}

	mb_create_disc_id(disc, buf);

	printf("DiscID: %s\n", buf);


	url = mb_create_submission_url(disc);

	printf("Submit: %s\n", url);

	free(url);
	free(disc);

	return 0;
}
