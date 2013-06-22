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

	announce("discid_get_version_string");
	evaluate(strlen(discid_get_version_string()) > 0);

	/* TODO
	 * test access with/without initialization doesn't fail
	 */

	announce("discid_new");
	d = discid_new();
	evaluate(d != NULL);

	announce("discid_free");
	discid_free(d);
	evaluate(1); /* only segfaults etc. would "show" */
	
	return !test_result();
}

/* EOF */
