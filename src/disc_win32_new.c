/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2007-2008 Lukas Lalinsky
   
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

#include <windows.h>
#include <string.h>
#include <stdio.h>
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#include "discid/discid.h"
#include "discid/discid_private.h"

#define CD_FRAMES     75
#define XA_INTERVAL		((60 + 90 + 2) * CD_FRAMES)

#define IOCTL_CDROM_READ_TOC         0x24000
#define IOCTL_CDROM_READ_Q_CHANNEL   0x2402c
#define IOCTL_CDROM_GET_LAST_SESSION 0x24038

typedef struct {
	UCHAR  Reserved;
	UCHAR  Control : 4;
	UCHAR  Adr : 4;
	UCHAR  TrackNumber;
	UCHAR  Reserved1;
	UCHAR  Address[4];
} TRACK_DATA;

typedef struct {
	UCHAR  Length[2];
	UCHAR  FirstTrack;
	UCHAR  LastTrack;
	TRACK_DATA  TrackData[100];
} CDROM_TOC;

typedef struct _CDROM_TOC_SESSION_DATA {
	UCHAR  Length[2];
	UCHAR  FirstCompleteSession;
	UCHAR  LastCompleteSession;
	TRACK_DATA  TrackData[1];
} CDROM_TOC_SESSION_DATA;

#define IOCTL_CDROM_SUB_Q_CHANNEL    0x00
#define IOCTL_CDROM_CURRENT_POSITION 0x01
#define IOCTL_CDROM_MEDIA_CATALOG    0x02
#define IOCTL_CDROM_TRACK_ISRC       0x03

typedef struct _CDROM_SUB_Q_DATA_FORMAT {
	UCHAR Format;
	UCHAR Track;
} CDROM_SUB_Q_DATA_FORMAT;

typedef struct _SUB_Q_HEADER {
	UCHAR  Reserved;
	UCHAR  AudioStatus;
	UCHAR  DataLength[2];
} SUB_Q_HEADER;

typedef struct _SUB_Q_MEDIA_CATALOG_NUMBER {
	SUB_Q_HEADER  Header;
	UCHAR  FormatCode;
	UCHAR  Reserved[3];
	UCHAR  Reserved1 : 7;
	UCHAR  Mcval :1;
	UCHAR  MediaCatalog[15];
} SUB_Q_MEDIA_CATALOG_NUMBER;

typedef struct _SUB_Q_TRACK_ISRC {
	SUB_Q_HEADER  Header;
	UCHAR  FormatCode;
	UCHAR  Reserved0;
	UCHAR  Track;
	UCHAR  Reserved1;
	UCHAR  Reserved2 : 7;
	UCHAR  Tcval : 1;
	UCHAR  TrackIsrc[15];
} SUB_Q_TRACK_ISRC;

typedef union _SUB_Q_CHANNEL_DATA {
	/*SUB_Q_CURRENT_POSITION  CurrentPosition;*/
	SUB_Q_MEDIA_CATALOG_NUMBER  MediaCatalog;
	SUB_Q_TRACK_ISRC  TrackIsrc;
} SUB_Q_CHANNEL_DATA;

int AddressToSectors(UCHAR address[4])
{
	return address[1] * 4500 + address[2] * 75 + address[3];
}

static void read_disc_mcn(HANDLE hDevice, mb_disc_private *disc)
{
	DWORD dwReturned;
	BOOL bResult;
	CDROM_SUB_Q_DATA_FORMAT format;
	SUB_Q_CHANNEL_DATA data;

	format.Track = 0;
	format.Format = IOCTL_CDROM_MEDIA_CATALOG;
	bResult = DeviceIoControl(hDevice, IOCTL_CDROM_READ_Q_CHANNEL,
                              &format, sizeof(format),
                              &data, sizeof(data),
                              &dwReturned, NULL);
	if (bResult == FALSE) {
		fprintf(stderr, "Warning: Unable to read the disc's media catalog number.\n");
	}
	else {
		strncpy(disc->mcn, (char *) data.MediaCatalog.MediaCatalog,
			MCN_STR_LENGTH);
	}
}

static void read_disc_isrc(HANDLE hDevice, mb_disc_private *disc, int track)
{
	DWORD dwReturned;
	BOOL bResult;
	CDROM_SUB_Q_DATA_FORMAT format;
	SUB_Q_CHANNEL_DATA data;

	format.Track = track;
	format.Format = IOCTL_CDROM_TRACK_ISRC;
	bResult = DeviceIoControl(hDevice, IOCTL_CDROM_READ_Q_CHANNEL,
                              &format, sizeof(format),
                              &data, sizeof(data),
                              &dwReturned, NULL);
	if (bResult == FALSE) {
		fprintf(stderr, "Warning: Unable to read the international standard recording code (ISRC) for track %i\n", track);
	}
	else {
		strncpy(disc->isrc[track], (char *) data.TrackIsrc.TrackIsrc,
			ISRC_STR_LENGTH);
	}
}

int mb_disc_has_feature_unportable(enum discid_feature feature) {
	switch(feature) {
		case DISCID_FEATURE_READ:
		case DISCID_FEATURE_MCN:
		case DISCID_FEATURE_ISRC:
			return 1;
		default:
			return 0;
	}
}


int mb_disc_read_unportable_nt(mb_disc_private *disc, const char *device, unsigned int features)
{
	HANDLE hDevice;
	DWORD dwReturned;
	BOOL bResult;
	CDROM_TOC toc;
	CDROM_TOC_SESSION_DATA session;
	char filename[128], *colon;
	int i, len;

	strcpy(filename, "\\\\.\\");
	len = strlen(device);
	colon = strchr(device, ':');
	if (colon) {
		len = colon - device + 1;
	}
	strncat(filename, device, len > 120 ? 120 : len);

	hDevice = CreateFile(filename, GENERIC_READ,
	                     FILE_SHARE_READ | FILE_SHARE_WRITE, 
	                     NULL, OPEN_EXISTING, 0, NULL);	
	if (hDevice == INVALID_HANDLE_VALUE) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			"couldn't open the CD audio device");
		return 0;
	}	

	bResult = DeviceIoControl(hDevice, IOCTL_CDROM_GET_LAST_SESSION,
	                          NULL, 0,
	                          &session, sizeof(session),
	                          &dwReturned, NULL);
	if (bResult == FALSE) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
		         "error while reading the CD TOC");
		CloseHandle(hDevice);
		return 0;
	}

	bResult = DeviceIoControl(hDevice, IOCTL_CDROM_READ_TOC,
	                          NULL, 0,
	                          &toc, sizeof(toc),
	                          &dwReturned, NULL);
	if (bResult == FALSE) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
		         "error while reading the CD TOC");
		CloseHandle(hDevice);
		return 0;
	}

	if (features & DISCID_FEATURE_MCN) {
		read_disc_mcn(hDevice, disc);
	}

	disc->first_track_num = toc.FirstTrack;
	disc->last_track_num = toc.LastTrack;

	/* multi-session disc */
	if (session.FirstCompleteSession != session.LastCompleteSession) {
		disc->last_track_num = session.TrackData[0].TrackNumber - 1;
		disc->track_offsets[0] =
			AddressToSectors(toc.TrackData[disc->last_track_num].Address) -
			XA_INTERVAL;
	}
	else {
		disc->track_offsets[0] =
			AddressToSectors(toc.TrackData[disc->last_track_num].Address);
	}

	for (i = disc->first_track_num; i <= disc->last_track_num; i++) {
		disc->track_offsets[i] = AddressToSectors(toc.TrackData[i - 1].Address);
		if (features & DISCID_FEATURE_ISRC) {
			read_disc_isrc(hDevice, disc, i);
		}
	}

	CloseHandle(hDevice);
	return 1;
}

/* EOF */

