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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <windows.h>

#if defined(__CYGWIN__)
#include <ntddcdrm.h>
#include <ntddscsi.h>
#elif defined(__MINGW32__)
#include <ddk/ntddcdrm.h>
#include <ddk/ntddscsi.h>
#else
#include "ntddcdrm.h"
#include "ntddscsi.h"
#endif

#include "discid/discid.h"
#include "discid/discid_private.h"
#include "scsi.h"


#define MB_DEFAULT_DEVICE	"D:"

/* after that time a scsi command is considered timed out */
#define DEFAULT_TIMEOUT 30	/* in seconds */

#define GOOD 0x00	/* scsi status code for success */


static int AddressToSectors(UCHAR address[4])
{
	return address[1] * 4500 + address[2] * 75 + address[3];
}

static HANDLE create_device_handle(mb_disc_private *disc, const char *device)
{
	HANDLE hDevice;
	char filename[128], *colon;
	int len;

	strcpy(filename, "\\\\.\\");
	len = strlen(device);
	colon = strchr(device, ':');
	if (colon) {
		len = colon - device + 1;
	}
	strncat(filename, device, len > 120 ? 120 : len);

	/* We are not actually "writing" to the device,
	 * but we are sending scsi commands with raw ISRCs,
	 * which needs GENERIC_WRITE.
	 */
	hDevice = CreateFile(filename, GENERIC_READ | GENERIC_WRITE,
	                     FILE_SHARE_READ | FILE_SHARE_WRITE,
	                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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


char *mb_disc_get_default_device_unportable(void) {
	return MB_DEFAULT_DEVICE;
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
		toc->tracks[i].address = AddressToSectors(cd.TrackData[i - 1].Address) - 150;
		toc->tracks[i].control = cd.TrackData[i - 1].Control;
	}

	/* Lead-out is stored after the last track */
	toc->tracks[0].address = AddressToSectors(cd.TrackData[toc->last_track_num].Address) - 150;
	toc->tracks[0].control = cd.TrackData[toc->last_track_num].Control;

	return 1;
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device,
			    unsigned int features) {
	mb_disc_toc toc;
	mb_scsi_handle handle;
	int i;

	memset(&handle, 0, sizeof handle);

	if ( !mb_disc_winnt_read_toc(disc, &toc, device) )
		return 0;

	if ( !mb_disc_load_toc(disc, &toc) )
		return 0;

	handle.hDevice = create_device_handle(disc, device);

	if (features & DISCID_FEATURE_MCN) {
		read_disc_mcn(handle.hDevice, disc);
	}

	for (i = disc->first_track_num; i <= disc->last_track_num; i++) {
		if (features & DISCID_FEATURE_ISRC) {
			//read_disc_isrc(hDevice, disc, i);
			mb_scsi_read_track_isrc_raw(handle, disc, i);
		}
	}

	CloseHandle(handle.hDevice);
	return 1;
}

mb_scsi_status mb_scsi_cmd_unportable(mb_scsi_handle handle,
			   unsigned char *cmd, int cmd_len,
			   unsigned char *data, int data_len) {
	SCSI_PASS_THROUGH_DIRECT sptd;
	DWORD bytes_returned = 0;
	int return_value;

	memset(&sptd, 0, sizeof sptd);
	sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
	sptd.DataIn = SCSI_IOCTL_DATA_IN;
	sptd.TimeOutValue = DEFAULT_TIMEOUT;
	sptd.DataBuffer = data;		/* a pointer */
	sptd.DataTransferLength = data_len;
	sptd.CdbLength = cmd_len;

	/* The command is a buffer, not a pointer.
	 * So we have to copy our buffer.
	 * The size of this buffer is not documented in MSDN,
	 * but in the include file defined as uchar[16].
	 */
	assert(cmd_len <= sizeof sptd.Cdb);
	memcpy(sptd.Cdb, cmd, cmd_len);

	/* the sptd struct is used for input and output -> listed twice
	 * We don't use bytes_returned, but this cannot be NULL in this case */
	return_value = DeviceIoControl(handle.hDevice,
				IOCTL_SCSI_PASS_THROUGH_DIRECT,
				&sptd, sizeof sptd, &sptd, sizeof sptd,
				&bytes_returned, NULL);

	if (return_value == 0) {
		/* failure */
		fprintf(stderr, "DeviceIoControl error: %ld\n", GetLastError());
		return IO_ERROR;
	} else {
		/* success of DeviceIoControl */

		/* check for potentially informative success codes
		 * 1 seems to be what is mostly returned */
		if (return_value != 1) {
			fprintf(stderr, "DeviceIoControl return value: %d\n",
					return_value);
			/* no actual error, but possibly informative */
		}

		/* check scsi status */
		if (sptd.ScsiStatus != GOOD) {
			fprintf(stderr, "scsi status: %d\n", sptd.ScsiStatus);
			return STATUS_ERROR;
		} else if (data_len > 0 && bytes_returned == 0) {
			/* not receiving data, when requested */
			fprintf(stderr, "data requested, but none returned\n");
			return NO_DATA_RETURNED;
		} else {
			return SUCCESS;
		}
	}
}

/* EOF */
