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
#include <string.h>
#include <discid/discid.h>
#include "discid/discid_private.h"


int main(int argc, char *argv[]) {
	DiscId *d = discid_new();
	mb_disc_private *disc = (mb_disc_private *) d;
	char *tmp, *expected;
	int ntests = 0, nok = 0;

	disc->first_track_num = 1;
	disc->last_track_num = 22;
	disc->track_offsets[0] = 303602;
	disc->track_offsets[1] = 150;
	disc->track_offsets[2] = 9700;
	disc->track_offsets[3] = 25887;
	disc->track_offsets[4] = 39297;
	disc->track_offsets[5] = 53795;
	disc->track_offsets[6] = 63735;
	disc->track_offsets[7] = 77517;
	disc->track_offsets[8] = 94877;
	disc->track_offsets[9] = 107270;
	disc->track_offsets[10] = 123552;
	disc->track_offsets[11] = 135522;
	disc->track_offsets[12] = 148422;
	disc->track_offsets[13] = 161197;
	disc->track_offsets[14] = 174790;
	disc->track_offsets[15] = 192022;
	disc->track_offsets[16] = 205545;
	disc->track_offsets[17] = 218010;
	disc->track_offsets[18] = 228700;
	disc->track_offsets[19] = 239590;
	disc->track_offsets[20] = 255470;
	disc->track_offsets[21] = 266932;
	disc->track_offsets[22] = 288750;
	disc->success = 1;
	
	/* MusicBrainz DiscID */
	printf("Testing discid_get_id ... ");
	tmp = discid_get_id(d);
	expected = "xUp1F2NkfP8s8jaeFn_Av3jNEI4-";
	if ( strcmp(tmp, expected) == 0 ) {
		printf("OK\n");
		nok++;
	}
	else {
		printf("Failed\n");
		printf("  Expected : %s\n", expected);
		printf("  Actual   : %s\n", tmp);
	}
	ntests++;

	/* FreeDB DiscID */
	printf("Testing discid_get_freedb_id ... ");
	tmp = discid_get_freedb_id(d);
	expected = "370fce16";
	if ( strcmp(tmp, expected) == 0 ) {
		printf("OK\n");
		nok++;
	}
	else {
		printf("Failed\n");
		printf("  Expected : %s\n", expected);
		printf("  Actual   : %s\n", tmp);
	}
	ntests++;

	/* MusicBrainz web submit URL */
	printf("Testing discid_get_submission_url ... ");
	tmp = discid_get_submission_url(d);
	expected = "http://mm.musicbrainz.org/bare/cdlookup.html?id=xUp1F2NkfP8s8jaeFn_Av3jNEI4-&tracks=22&toc=1+22+303602+150+9700+25887+39297+53795+63735+77517+94877+107270+123552+135522+148422+161197+174790+192022+205545+218010+228700+239590+255470+266932+288750";
	if ( strcmp(tmp, expected) == 0 ) {
		printf("OK\n");
		nok++;
	}
	else {
		printf("Failed\n");
		printf("  Expected : %s\n", expected);
		printf("  Actual   : %s\n", tmp);
	}
	ntests++;

	printf("\n%d tests, %d passed, %d failed\n", ntests, nok, ntests - nok);
	
	discid_free(d);
	
	return ntests != nok;
}

/* EOF */
