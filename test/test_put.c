/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender
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
#include <string.h>

#include <discid/discid.h>
#include "test.h"


int main(int argc, char *argv[]) {
	DiscId *d;
	char *expected;
	int offsets[] = {
		303602,
		150, 9700, 25887, 39297, 53795, 63735, 77517, 94877, 107270,
		123552, 135522, 148422, 161197, 174790, 192022, 205545,
		218010, 228700, 239590, 255470, 266932, 288750,
	};

	d = discid_new();

	/* TODO
	 * test a put failure, as in with wrong input
	 */

	/* Setting TOC */
	announce("discid_put");
	evaluate(discid_put(d, 1, 22, offsets));

	/* MusicBrainz DiscID */
	announce("discid_get_id");
	evaluate(equal_str(discid_get_id(d),
			   "xUp1F2NkfP8s8jaeFn_Av3jNEI4-", ""));

	/* FreeDB DiscID */
	announce("discid_get_freedb_id");
	evaluate(equal_str(discid_get_freedb_id(d), "370fce16", ""));

	/* MusicBrainz web submit URL */
	announce("discid_get_submission_url");
	expected = "http://mm.musicbrainz.org/bare/cdlookup.html?id=xUp1F2NkfP8s8jaeFn_Av3jNEI4-&tracks=22&toc=1+22+303602+150+9700+25887+39297+53795+63735+77517+94877+107270+123552+135522+148422+161197+174790+192022+205545+218010+228700+239590+255470+266932+288750";
	evaluate(equal_str(discid_get_submission_url(d), expected, ""));


	discid_free(d);
	
	return !test_result();
}

/* EOF */
