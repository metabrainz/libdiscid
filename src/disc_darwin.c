/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2006 Robert Kaye
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

     $Id: mb_darwin.cpp,v 1.4 2005/10/28 22:05:27 robert Exp $

----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <paths.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/storage/IOCDMedia.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOCDTypes.h>
#include <IOKit/storage/IOCDMediaBSDClient.h>
#include <IOKit/storage/IOMediaBSDClient.h>

#include <CoreFoundation/CoreFoundation.h>

#include "discid/discid.h"
#include "discid/discid_private.h"

#define TOC_BUFFER_LEN 2048
#define MAXPATHLEN     1024

static char defaultDevice[MAXPATHLEN] = "\0";

static kern_return_t find_ejectable_cd_media( io_iterator_t *mediaIterator )
{
    mach_port_t         masterPort;
    kern_return_t       kernResult;
    CFMutableDictionaryRef   classesToMatch;

    kernResult = IOMasterPort( MACH_PORT_NULL, &masterPort );
    if ( kernResult != KERN_SUCCESS )
        return kernResult;

    // CD media are instances of class kIOCDMediaClass.
    classesToMatch = IOServiceMatching( kIOCDMediaClass );
    if ( classesToMatch != NULL )
    {
        // Each IOMedia object has a property with key kIOMediaEjectableKey
        // which is true if the media is indeed ejectable. So add this
        // property to the CFDictionary for matching.
        CFDictionarySetValue( classesToMatch, CFSTR( kIOMediaEjectableKey ), kCFBooleanTrue );
    }

    return IOServiceGetMatchingServices( masterPort, classesToMatch, mediaIterator );
}

static kern_return_t get_device_file_path( io_iterator_t mediaIterator, char *deviceFilePath, CFIndex maxPathSize )
{
    io_object_t nextMedia;
    kern_return_t kernResult = KERN_FAILURE;
 
    *deviceFilePath = '\0';
    nextMedia = IOIteratorNext( mediaIterator );
    if ( nextMedia )
    {
        CFTypeRef   deviceFilePathAsCFString;
        deviceFilePathAsCFString = IORegistryEntryCreateCFProperty(
                                nextMedia, CFSTR( kIOBSDNameKey ),
                                kCFAllocatorDefault, 0 );

       *deviceFilePath = '\0';
        if ( deviceFilePathAsCFString )
        {
            size_t devPathLength;
            strcpy( deviceFilePath, _PATH_DEV );
            // Add "r" before the BSD node name from the I/O Registry
            // to specify the raw disk node. The raw disk node receives
            // I/O requests directly and does not go through the
            // buffer cache.
            strcat( deviceFilePath, "r");
            devPathLength = strlen( deviceFilePath );
            if ( CFStringGetCString( deviceFilePathAsCFString,
                                     deviceFilePath + devPathLength,
                                     maxPathSize - devPathLength,
                                     kCFStringEncodingASCII ) )
                kernResult = KERN_SUCCESS;

            CFRelease( deviceFilePathAsCFString );
        }
    }
    IOObjectRelease( nextMedia );

    return kernResult;
}

static void read_disc_mcn(int fd, mb_disc_private *disc)
{
    dk_cd_read_mcn_t cd_read_mcn;
    bzero(&cd_read_mcn, sizeof(cd_read_mcn));
	
    if(ioctl(fd, DKIOCCDREADMCN, &cd_read_mcn) == -1) {
        fprintf(stderr, "Warning: Unable to read the disc's media catalog number.\n");
    } else {
        strncpy( disc->mcn, cd_read_mcn.mcn, MCN_STR_LENGTH );
    }
}

static void read_disc_isrc(int fd, mb_disc_private *disc, int track)
{
    dk_cd_read_isrc_t	cd_read_isrc;
    bzero(&cd_read_isrc, sizeof(cd_read_isrc));
    cd_read_isrc.track = track;
	
    if(ioctl(fd, DKIOCCDREADISRC, &cd_read_isrc) == -1) {
        fprintf(stderr, "Warning: Unable to read the international standard recording code (ISRC) for track %i\n", track);
        return;
    } else {
        strncpy( disc->isrc[track], cd_read_isrc.isrc, ISRC_STR_LENGTH );
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


char *mb_disc_get_default_device_unportable(void) 
{
    kern_return_t kernResult;
    io_iterator_t mediaIterator;

    *defaultDevice = 0;

    kernResult = find_ejectable_cd_media( &mediaIterator );
    if ( kernResult != KERN_SUCCESS )
        return "";

    kernResult = get_device_file_path( mediaIterator, defaultDevice, MAXPATHLEN - 1);
    if ( kernResult != KERN_SUCCESS )
        return "";

    return defaultDevice;
}

int mb_disc_read_unportable(mb_disc_private *disc, const char *device, unsigned int features) 
{
	int fd;
	int i;
	dk_cd_read_toc_t toc;
	CDTOC *cdToc;

	if (device == NULL || *device == 0) {
		device = mb_disc_get_default_device_unportable();
	}

	if (!*device) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			 "No CD-ROMs found. Please insert a disc and try again."
			);
		return 0;
	}
  
	fd = open(device, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			 "Cannot open '%s'", device);
		return 0;
	}
  
	memset(&toc, 0, sizeof(toc));
	toc.format = kCDTOCFormatTOC;
	toc.formatAsTime = 0;
	toc.buffer = (char *)malloc(TOC_BUFFER_LEN);
	toc.bufferLength = TOC_BUFFER_LEN;
	if (ioctl(fd, DKIOCCDREADTOC, &toc) < 0 ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			 "Cannot read TOC from '%s'", device);
		free(toc.buffer);
		return 0;
	}
	if ( toc.bufferLength < sizeof(CDTOC) ) {
		snprintf(disc->error_msg, MB_ERROR_MSG_LENGTH,
			 "Short TOC was returned from '%s'", device);
		free(toc.buffer);
		return 0;
	}

	// Read in the media catalogue number
	if (features & DISCID_FEATURE_MCN) {
		read_disc_mcn(fd, disc);
	}

	cdToc = (CDTOC *)toc.buffer;
	int numDesc = CDTOCGetDescriptorCount(cdToc);

	int numTracks = 0;
	for(i = 0; i < numDesc; i++) {
		CDTOCDescriptor *desc = &cdToc->descriptors[i];

		if (desc->session > 1) {
			continue;
		}

		if (desc->point == 0xA2 && desc->adr == 1) {
			disc->track_offsets[0] = CDConvertMSFToLBA(desc->p)
						 + 150;
		}

		if (desc->point <= 99 && desc->adr == 1) {
			disc->track_offsets[1 + numTracks] = CDConvertMSFToLBA(
								desc->p) + 150;

			// Read in the IRSC codes for tracks
			if (features & DISCID_FEATURE_ISRC) {
				read_disc_isrc(fd, disc, 1 + numTracks);
			}

			numTracks++;
		}
	}
	disc->first_track_num = 1;
	disc->last_track_num = numTracks;


	close(fd);
	free(toc.buffer);
  
	return 1;
}
