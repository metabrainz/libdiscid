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

#define MB_DEFAULT_DEVICE	"D:" /* FIXME */

#define IOCTL_CDROM_READ_TOC         0x24000

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


int AddressToSectors(UCHAR address[4])
{
	return address[1] * 4500 + address[2] * 75 + address[3];
}


char *mb_disc_get_default_device_unportable(void) {
	return MB_DEFAULT_DEVICE;
} 


int mb_disc_winnt_read_toc(mb_disc_private *disc, mb_disc_toc *toc, const char *device)
{
	HANDLE hDevice;
	DWORD dwReturned;
	BOOL bResult;
	CDROM_TOC cd;
	char filename[128], *colon;
	int i, len;

	strcpy(filename, "\\\\.\\");
	len = strlen(device);
	if (colon = strchr(device, ':')) {
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

	bResult = DeviceIoControl(hDevice, IOCTL_CDROM_READ_TOC,
	                          NULL, 0,
	                          &cd, sizeof(cd),
	                          &dwReturned, NULL);
	if (bResult == FALSE) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
		         "error while reading the CD TOC");
		CloseHandle(hDevice);
		return 0;
	}

	CloseHandle(hDevice);

	toc->first_track_num = cd.FirstTrack;
	toc->last_track_num = cd.LastTrack;

	/* Get info about all tracks */
	for (i = toc->first_track_num; i <= toc->last_track_num; i++) {
		toc->tracks[i].address = AddressToSectors(cd.TrackData[i - 1].Address) - 150;
		toc->tracks[i].control = cd.TrackData[i - 1].Control;
	}

	/* Lead-out is stored after the last track */
	toc->tracks[0].address = AddressToSectors(cd.TrackData[toc->last_track_num].Address) - 150;
	toc->tracks[0].control = cd.TrackData[toc->last_track_num].Control;

	return 1;
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device) {
	mb_disc_toc toc;

	if ( !mb_disc_winnt_read_toc(disc, &toc, device) )
		return 0;

	if ( !mb_disc_load_toc(disc, &toc) )
		return 0;
		
	return 1;
}

/* EOF */

