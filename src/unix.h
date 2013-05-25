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

#include "discid/discid_private.h"


/*
 * Read the TOC header from disc
 *
 * THIS FUNCTION HAS TO BE IMPLEMENTED FOR THE PLATFORM
 */
LIBDISCID_INTERNAL int mb_disc_unix_read_toc_header(int fd, mb_disc_toc *toc);

/*
 * Read a TOC entry for a certain track from disc
 *
 * THIS FUNCTION HAS TO BE IMPLEMENTED FOR THE PLATFORM
 */
LIBDISCID_INTERNAL int mb_disc_unix_read_toc_entry(int fd, int track_num, mb_disc_toc_track *track);


/*
 * This function is implemented in unix.c and can be used
 * after the above functions are implemented on the platform.
 */
LIBDISCID_INTERNAL int mb_disc_unix_read_toc(mb_disc_private *disc, mb_disc_toc *toc,
			  const char *device);

/*
 * utility function to try opening the device with open()
 */
LIBDISCID_INTERNAL int mb_disc_unix_open(mb_disc_private *disc, const char *device);
