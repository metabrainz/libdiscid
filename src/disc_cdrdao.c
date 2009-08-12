/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2009 Lukas Lalinsky

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

----------------------------------------------------------------------------*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "discid/discid_private.h"

static int strip_nl(char *line) {
	int length = strlen(line);
	while ( length > 0 && line[length - 1] == '\n' || line[length - 1] == '\r' ) {
		line[length - 1] = '\0';
		--length;
	}
	return length;
}

int mb_disc_read_cdrdao(mb_disc_private *disc, const char *device) {
	int ret, track = -1;
	char command[256], *tocname;
	FILE *tocfile;

	tocname = tempnam(NULL, "libdiscid");

	/* run cdrdao to get ISRC/MCN information */
	snprintf(command, 256, "cdrdao read-toc --device %s --fast-toc %s 2>/dev/null >/dev/null", device, tocname);
	ret = system(command);
	if ( WEXITSTATUS(ret) != 0 )
		goto cleanup;

	tocfile = fopen(tocname, "rt");
	if ( !tocfile )
		goto cleanup;

	/* parse the TOC file */
	while (1) {
		char line[1024];
		if ( !fgets(line, 1024, tocfile) )
			break;
		if ( !strncmp(line, "// Track ", 9) ) {
			track = atoi(&line[9]);
		}
		else if ( !strncmp(line, "ISRC \"", 6) ) {
			if ( track >= 1 && track <= 99 ) {
				int length = strip_nl(line);
				if ( length == 19 && line[length -1 ] == '"' ) {
					strncpy(disc->isrc[track], &line[6], ISRC_STR_LENGTH);
				}
			}
		}
		else if ( !strncmp(line, "CATALOG \"", 9) ) {
			int length = strip_nl(line);
			if ( length <= 10 + MCN_STR_LENGTH && line[length - 1] == '"' ) {
				strncpy(disc->mcn, &line[9], length - 10);
			}
		}
	}

	fclose(tocfile);

cleanup:
	unlink(tocname);
	free(tocname);

	return 1;
}

/* EOF */
