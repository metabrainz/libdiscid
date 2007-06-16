/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2007 Lukas Lalinsky
   
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
#include "discid/discid_private.h"

#define CD_FRAMES     75
#define XA_INTERVAL		((60 + 90 + 2) * CD_FRAMES)

#define IOCTL_CDROM_READ_TOC         0x24000
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

int AddressToSectors(UCHAR address[4])
{
	return address[1] * 4500 + address[2] * 75 + address[3];
}

int mb_disc_read_unportable_nt(mb_disc_private *disc, const char *device)
{
	HANDLE hDevice;
	DWORD dwReturned;
	BOOL bResult;
	CDROM_TOC toc;
	CDROM_TOC_SESSION_DATA session;
	char filename[128];
	int i;

	snprintf(filename, 128, "\\\\.\\%s", device);

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

	CloseHandle(hDevice);

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
	}

	return 1;
}

/* EOF */

