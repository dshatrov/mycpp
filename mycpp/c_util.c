/* This is a workaround for an ugly API bug in the GNU C library:
 * portable strerror_r() function requires _GNU_SOURCE macro not
 * to be defined, but g++ defines it unconditionally.
 * The consequences of undef'ing it before including
 * <string.h> are uncertain and may well be fatal. */

#ifdef _GNU_SOURCE
#undef _GNU_SOURCE
#endif

/* See man strerror_r */
#define _XOPEN_SOURCE 600
#include <string.h>

int
_mycpp_strerror_r (int     errnum,
		   char   *buf,
		   size_t  buflen)
{
#ifndef PLATFORM_WIN32
    return strerror_r (errnum, buf, buflen);
#else
    // TODO
    if (buf != NULL && buflen > 0)
	buf [0] = 0;
    return 0;
#endif
}

