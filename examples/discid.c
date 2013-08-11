/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender, Laurent Monin
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
#ifdef _MSC_VER
#define snprintf _snprintf
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <discid/discid.h>

#ifndef DISCID_HAVE_SPARSE_READ
#define discid_read_sparse(disc, dev, i) discid_read(disc, dev)
#endif

#define SECTORS_PER_SECOND 75
#define ROUND_SECONDS 0

/* Convert a number of sectors to a human-readable time in hours,minutes,seconds
 * If round is true, seconds will be rounded to nearest,
 * else 1/100 seconds time precision is used.
 * If duration is over one hour, hours will be added in front of the string
 * Examples:
 *  33284 sectors with round=0 -> ( 7:23.79)
 *  33284 sectors with round=1 -> ( 7:24)
 * 356163 sectors with round=1 -> ( 1:19:09)
 * 356163 sectors with round=0 -> ( 1:19:08.84)
 *
 * Result is written to buf
 */
void sectors_to_time(int sectors, int round, char *buf, size_t bufsize) {
	float duration_in_secs = (float) sectors / SECTORS_PER_SECOND;
	int hours = (int) duration_in_secs / 3600;
	int minutes = (int) (duration_in_secs - hours * 3600) / 60;
	float seconds = duration_in_secs - (hours * 3600 + minutes * 60);

	if (round) {
		int seconds_rounded = (int) (seconds + 0.5);
		if (hours > 0) {
			snprintf(buf, bufsize, "%d:%02d:%02d",
				 hours, minutes, seconds_rounded);
		} else {
			snprintf(buf, bufsize, "  %2d:%02d",
				 minutes, seconds_rounded);
		}
	} else {
		if (hours > 0) {
			snprintf(buf, bufsize, "%d:%02d:%05.2f",
				 hours, minutes, seconds);
		} else {
			snprintf(buf, bufsize, "  %2d:%05.2f",
				 minutes, seconds);
		}
	}
}

int main(int argc, char *argv[]) {
	int i, first_track, last_track;
	char *device = NULL;
	char time_str[14];
	int sectors;
	DiscId *disc;

	disc = discid_new();

	/* If we have an argument, use it as the device name */
	if (argc > 1) {
		device = argv[1];
	} else {
		/* this will use discid_get_default_device() internally */
		device = NULL;
	}

	if (discid_read_sparse(disc, device, 0) == 0) {
		fprintf(stderr, "Error: %s\n", discid_get_error_msg(disc));
		discid_free(disc);
		return 1;
	}

	printf("DiscID        : %s\n", discid_get_id(disc));
	printf("FreeDB DiscID : %s\n", discid_get_freedb_id(disc));

	first_track = discid_get_first_track_num(disc);
	last_track = discid_get_last_track_num(disc);
	printf("First track   : %d\n", first_track);
	printf("Last track    : %d\n", last_track);

	sectors = discid_get_sectors(disc);
	sectors_to_time(sectors, ROUND_SECONDS, time_str, sizeof time_str);
	printf("Length        : %d sectors (%s)\n", sectors, time_str);

	for ( i = first_track; i <= last_track; i++ ) {
		sectors = discid_get_track_length(disc, i);
		sectors_to_time(sectors, ROUND_SECONDS,
				time_str, sizeof time_str);
		printf("Track %-2d      : %8d %8d (%s)\n",
				i, discid_get_track_offset(disc, i),
				sectors, time_str);
	}

	printf("Submit via    : %s\n", discid_get_submission_url(disc));

	discid_free(disc);

	return 0;
}

/* EOF */
