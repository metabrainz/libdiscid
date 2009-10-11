/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2006 Lukas Lalinsky

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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "discid/discid_private.h"

#define XA_INTERVAL		((60 + 90 + 2) * 75)
#define DATA_TRACK		0x04

int snprintf(char *str, size_t size, const char *format, ...);


int mb_disc_load_toc(mb_disc_private *disc, mb_disc_toc *toc)  {
	int first_audio_track, last_audio_track, data_tracks, i;
	mb_disc_toc_track *track;

	if ( toc->first_track_num < 1 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"invalid CD TOC - first track number must be 1 or higher");
		return 0;
	}

	if ( toc->last_track_num < 1 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"invalid CD TOC - last track number must be 99 or lower");
		return 0;
	}

	/* scan the TOC for audio tracks */
	first_audio_track = -1;
	last_audio_track = -1;
	data_tracks = 0;
	for ( i = toc->first_track_num; i <= toc->last_track_num; i++ ) {
		track = &toc->tracks[i];
		if ( track->control & DATA_TRACK ) {
			data_tracks += 1;
		}
		else {
			/* if this is the first audio track we see, set first_audio_track */
			if ( first_audio_track < 0 ) {
				if ( data_tracks ) {
					snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
						"invalid CD TOC - data tracks before audio tracks are not supported");
					return 0;
				}
				first_audio_track = i;
			}
			/* if we have not seen any data tracks yet, set last_audio_track */
			if ( !data_tracks ) {
				last_audio_track = i;
			}
		}
	}

	disc->first_track_num = first_audio_track;
	disc->last_track_num = last_audio_track;

	/* get offsets for all found data tracks */
	for ( i = first_audio_track; i <= last_audio_track; i++ ) {
		track = &toc->tracks[i];
		disc->track_offsets[i] = track->address + 150;
	}

	/* the last audio track is not the last track on the CD, use the offset
	   of the next data track as the "lead-out" offset */
	if ( last_audio_track < toc->last_track_num ) {
		track = &toc->tracks[last_audio_track + 1];
		disc->track_offsets[0] = track->address - XA_INTERVAL + 150;
	}
	/* use the regular lead-out track */
	else {
		track = &toc->tracks[0];
		disc->track_offsets[0] = track->address + 150;
	}

	return 1;
}

/* EOF */
