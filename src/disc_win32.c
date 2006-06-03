/*
 * Add required includes ...
 */

#include "discid_private.h"


#define MB_DEFAULT_DEVICE	"TODO"


char *mb_disc_get_default_device_unportable(void) {
	return MB_DEFAULT_DEVICE;
}


/*
 * disc and device are not NULL (mb_disc_read() checks that for you).
 */
int mb_disc_read_unportable(mb_disc_private *disc, char *device) {

	return 0; /* error */
}

/* EOF */
