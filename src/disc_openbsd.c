/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2009 Anton Yabchinskiy
   Copyright (C) 2006 Matthias Friedrich
   Copyright (C) 2000 Robert Kaye
   Copyright (C) 1999 Marc E E van Woerkom

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

#include <sys/types.h>
#include <sys/cdio.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "discid/discid.h"
#include "discid/discid_private.h"

#define MB_DEFAULT_DEVICE   "/dev/rcd0c"

#ifdef __NetBSD__
    #define CD_TRACK_LEADOUT    0xAA
    #ifdef __i386__
        #define MB_DEFAULT_DEVICE   "/dev/rcd0d"
    #endif/* __i386__ */
#endif/* __NetBSD__ */

static int
read_toc_entry (int fd, int track, struct cd_toc_entry *te)
{
    struct ioc_read_toc_entry   rte;

    rte.address_format = CD_LBA_FORMAT;
    rte.data           = te;
    rte.data_len       = sizeof (*te);
    rte.starting_track = track;

    if (ioctl (fd, CDIOREADTOCENTRYS, &rte) == -1) {
        return errno;
    }

    return 0;
}

static inline int
is_data_track (const struct cd_toc_entry *te)
{
    return (te->control & 0x4) != 0;
}

int
mb_disc_read_unportable (mb_disc_private *disc, const char *device, unsigned int features)
{
    struct cd_toc_entry     te;
    struct ioc_toc_header   th;
    int                     fd, i, rc;

    fd = open (device, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        snprintf (disc->error_msg, MB_ERROR_MSG_LENGTH,
            "could not open device '%s' (errno %i, '%s')", device, errno,
            strerror (errno));
        return 0;
    }

    rc = ioctl (fd, CDIOREADTOCHEADER, &th);
    if (rc == -1) {
        snprintf (disc->error_msg, MB_ERROR_MSG_LENGTH,
            "could not read table of contents (errno %i, '%s')", errno,
            strerror (errno));
        close (fd);
        return 0;
    }

    if (th.ending_track == 0) {
        snprintf (disc->error_msg, MB_ERROR_MSG_LENGTH, "disc has no tracks");
        close (fd);
        return 0;
    }

    disc->first_track_num = th.starting_track;
    disc->last_track_num  = th.ending_track;

    rc = read_toc_entry (fd, CD_TRACK_LEADOUT, &te);
    if (rc != 0) {
        snprintf (disc->error_msg, MB_ERROR_MSG_LENGTH,
            "could not read TOC entry for lead-out track (errno %i, '%s')",
            rc, strerror (rc));
        close (fd);
        return 0;
    }
    disc->track_offsets [0] = (int) te.addr.lba + 150;

    for (i = th.starting_track; i <= th.ending_track; ++i) {
        rc = read_toc_entry (fd, i, &te);
        if (rc != 0) {
            snprintf (disc->error_msg, MB_ERROR_MSG_LENGTH,
                "could not read TOC entry for track %i (errno %i, '%s')", i,
                rc, strerror (rc));
            close (fd);
            return 0;
        }

        if (is_data_track (&te)) {
            if (i == th.starting_track) {
                snprintf (disc->error_msg, MB_ERROR_MSG_LENGTH,
                    "disc has no audio tracks");
                close (fd);
                return 0;
            }
            break;
        }

        disc->track_offsets [i] = (int) te.addr.lba + 150;
    }

    close (fd);

    return 1;
}

char *
mb_disc_get_default_device_unportable (void)
{
    return MB_DEFAULT_DEVICE;
}

int
mb_disc_has_feature_unportable(enum discid_feature feature) {
	switch(feature) {
		case DISCID_FEATURE_READ:
			return 1;
		default:
			return 0;
	}
}
