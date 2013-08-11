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

----------------------------------------------------------------------------*/

#ifdef _MSC_VER
#define snprintf _snprintf
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <string.h>
#include <stdio.h>

#if defined(__CYGWIN__)
#include <ntddcdrm.h>
#elif defined(__MINGW32__)
#include <ddk/ntddcdrm.h>
#else
#include "ntddcdrm.h"
#endif

#include "discid/discid.h"
#include "discid/discid_private.h"


#define MB_DEFAULT_DEVICE	"D:"
#define MAX_DEV_LEN 3

#if defined(_MSC_VER)
#	define THREAD_LOCAL __declspec(thread)
#elif (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#	define THREAD_LOCAL __thread
#else
#	define THREAD_LOCAL
#endif

static THREAD_LOCAL char default_device[MAX_DEV_LEN] = "\0"; 

static int address_to_sectors(UCHAR address[4])
{
	return address[1] * 4500 + address[2] * 75 + address[3];
}

static HANDLE create_device_handle(mb_disc_private *disc, const char *device)
{
	HANDLE hDevice;
	char filename[128];
	const char* colon;
	int len;

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

	return hDevice;
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

char *get_nth_device(int number) {
	int i, counter = 0;
	char device[MAX_DEV_LEN];
	DWORD mask = GetLogicalDrives();
	
	for (i = 0; i <= 25; i++) {
		if (mask >> i & 1) {
			snprintf(device, MAX_DEV_LEN, "%c:", i + 'A');
			
			if (GetDriveType(device) == DRIVE_CDROM) {
				counter++;

				if (counter == number)
				{
					strncpy(default_device, device, MAX_DEV_LEN);
					return default_device;
				}
			}
		}
	}

	return NULL;
}

char *mb_disc_get_default_device_unportable(void) {
	char* device = get_nth_device(1);

	if (device == NULL)
	{
		return MB_DEFAULT_DEVICE;
	}

	return device;
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


int mb_disc_winnt_read_toc(mb_disc_private *disc, mb_disc_toc *toc, const char *device)
{
	HANDLE hDevice;
	DWORD dwReturned;
	BOOL bResult;
	CDROM_TOC cd;
	int i;

	hDevice = create_device_handle(disc, device);

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
		toc->tracks[i].address = address_to_sectors(cd.TrackData[i - 1].Address) - 150;
		toc->tracks[i].control = cd.TrackData[i - 1].Control;
	}

	/* Lead-out is stored after the last track */
	toc->tracks[0].address = address_to_sectors(cd.TrackData[toc->last_track_num].Address) - 150;
	toc->tracks[0].control = cd.TrackData[toc->last_track_num].Control;

	return 1;
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device,
			    unsigned int features) {
	mb_disc_toc toc;
	HANDLE hDevice;
	int i, device_number;

	device_number = (int) strtol(device, NULL, 10);

	if (device_number > 0) {
		device = get_nth_device(device_number);

		if (device == NULL)
		{
			snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
				"couldn't open the CD audio device");
			return 0;
		}
	}

	if ( !mb_disc_winnt_read_toc(disc, &toc, device) )
		return 0;

	if ( !mb_disc_load_toc(disc, &toc) )
		return 0;

	hDevice = create_device_handle(disc, device);

	if (features & DISCID_FEATURE_MCN) {
		read_disc_mcn(hDevice, disc);
	}

	for (i = disc->first_track_num; i <= disc->last_track_num; i++) {
		if (features & DISCID_FEATURE_ISRC) {
			read_disc_isrc(hDevice, disc, i);
		}
	}

	CloseHandle(hDevice);
	return 1;
}

/* EOF */
