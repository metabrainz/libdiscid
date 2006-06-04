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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA

     $Id$

--------------------------------------------------------------------------- */
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
