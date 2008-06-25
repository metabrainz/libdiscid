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

     $Id$

--------------------------------------------------------------------------- */
#include <stdio.h>
#include <discid/discid.h>


int main(int argc, char *argv[]) {
	DiscId *disc = discid_new();
	int i;
  char *device = NULL;

  /* If we have an argument, use it as the device name */
  if (argc > 1)
      device = argv[1];

	/* read the disc in the default disc drive */
	if ( discid_read(disc, device) == 0 ) {
		fprintf(stderr, "Error: %s\n", discid_get_error_msg(disc));
		return 1;
	}

  printf("MCN      : %s\n", discid_get_mcn(disc));

	for ( i = discid_get_first_track_num(disc);
			i <= discid_get_last_track_num(disc); i++ ) {

		printf("Track %-2d : %s\n", i, discid_get_track_isrc(disc, i));
	}

	discid_free(disc);

	return 0;
}

/* EOF */
