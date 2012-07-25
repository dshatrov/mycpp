/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006  Dmitry M. Shatrov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <errno.h>

/* Banning printf* functions.
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <glib.h>
#include <glib/gprintf.h>
*/

#include <stdlib.h>
#include <math.h>

#include <mycpp/util.h>
#include <mycpp/strutil.h>
#include <mycpp/io.h>

#ifdef PLATFORM_WIN32
#include <Windows.h>
#endif

extern "C" {

/* See c_util.c for the explanation. */
int
_mycpp_strerror_r (int     errnum,
		   char   *buf,
		   size_t  buflen);

}

namespace MyCpp {

#if 0
/* This could well be a simple wrapper for memcpy.
 * It just doesn't matter at the moment, and I do want to
 * have such a wrapper for API consistency. */
void
copyMemory (void          *_to,
	    const void    *_from,
	    unsigned long  len)
{
    /* Interestingly enough, I've spend about an hour digging through
     * C and C++ standards to find a proof of that it is correct
     * to cast a pointer to (any) type to void* and then to (unsigned) char*,
     * and use the resulting char* pointer. I didn't find any.
     * From what I've actually found on this question, the result
     * of such conversion is undefined. There is an example
     * of using memcpy() to copy object's contents, though, which shows
     * that authors of the standards probably had no malice intentions.
     * I'm a bit upset with this anyway. Please let me know if I've missed
     * something in the standards.
     *     - DMS, 07.08.11 */
    unsigned char *to = static_cast <unsigned char*> (_to);
    const unsigned char *from = static_cast <const unsigned char*> (_from);

    unsigned long i;
    for (i = 0; i < len; i++)
	to [i] = from [i];
}

void
zeroMemory (void          *_mem,
	    unsigned long  len)
{
    unsigned char *mem = static_cast <unsigned char*> (_mem);

    unsigned long i;
    for (i = 0; i < len; i++)
	mem [i] = 0;
}
#endif

#if 0
// These slow hand-written versions have been replaced with standard library
// calls. I trust them now.

void
copyMemory (MemoryDesc const &to,
	    ConstMemoryDesc const &from)
{
    abortIf (to.getLength () < from.getLength ());

    for (Size i = 0; i < from.getLength (); i++)
	to.getMemory () [i] = from.getMemory () [i];
}

void
copyMemory (MemoryDesc const &to,
	    ConstMemoryDesc const &from,
	    Size len)
{
    copyMemory (to, from.getRegion (0, len));
}

void
zeroMemory (MemoryDesc const &mem)
{
    for (Size i =0; i < mem.getLength (); i++)
	mem.getMemory () [i] = 0;
}

unsigned long
countStrLength (const char *str)
{
    if (str == NULL)
	abortIfReached ();

    unsigned long len = 0;

    while (str [len] != 0)
	len ++;

    return len;
}
#endif

/* TODO Rename this to stringsEqual () */
bool
compareStrings (const char *str1,
		const char *str2)
{
    unsigned long pos = 0;
    for (;;) {
	if (str1 [pos] == 0) {
	    if (str2 [pos] == 0)
		return true;

	    break;
	}

	if (str2 [pos] == 0)
	    break;

	if (str1 [pos] != str2 [pos])
	    break;

	pos ++;
    }

    return false;
}

/* TODO Rename this to compareStrings () */
ComparisonResult
orderStrings (const char *str1,
	      const char *str2)
{
    if (str1 == NULL || str1 [0] == 0) {
	if (str2 == NULL || str2 [0] == 0)
	    return ComparisonEqual;

	return ComparisonLesser;
    }

    if (str2 == NULL || str2 [0] == 0)
	return ComparisonGreater;

    unsigned long pos = 0;
    for (;;) {
	if (str1 [pos] == 0) {
	    if (str2 [pos] == 0)
		return ComparisonEqual;

	    return ComparisonLesser;
	}

	if (str2 [pos] == 0)
	    return ComparisonGreater;

	if (str1 [pos] > str2 [pos])
	    return ComparisonGreater;
	else
	if (str1 [pos] < str2 [pos])
	    return ComparisonLesser;

	pos ++;
    }

    /* Unreachable */
    return ComparisonEqual;
}

ComparisonResult
compareByteArrays (ConstMemoryDesc const &buf1,
		   ConstMemoryDesc const &buf2)
{
    if ((buf1.getMemory () == NULL && buf1.getLength () > 0) ||
	(buf2.getMemory () == NULL && buf2.getLength () > 0))
    {
	abortIfReached ();
    }

    unsigned long pos = 0;
    for (;;) {
	if (pos >= buf1.getLength ()) {
	    if (pos >= buf2.getLength ())
		return ComparisonEqual;

	    return ComparisonLesser;
	}

	if (pos >= buf2.getLength ())
	    return ComparisonGreater;

	if (buf1.getMemory () [pos] > buf2.getMemory () [pos])
	    return ComparisonGreater;
	else
	if (buf1.getMemory () [pos] < buf2.getMemory () [pos])
	    return ComparisonLesser;

	pos ++;
    }

    /* Unreachable */
    abortIfReached ();
    return ComparisonEqual;
}

ComparisonResult
compareByteArrayToString (ConstMemoryDesc const &buf,
			  const char *str)
{
    unsigned long pos = 0;
    for (;;) {
	if (pos >= buf.getLength ()) {
	    if (str [pos] == 0)
		return ComparisonEqual;

	    return ComparisonLesser;
	}

	if (str [pos] == 0)
	    return ComparisonGreater;

	if (buf.getMemory () [pos] > (unsigned char) str [pos])
	    return ComparisonGreater;
	else
	if (buf.getMemory () [pos] < (unsigned char) str [pos])
	    return ComparisonLesser;

	pos ++;
    }

    /* Unreachable */
    return ComparisonEqual;
}

bool
stringHasSuffix (ConstMemoryDesc const &str,
		 ConstMemoryDesc const &suffix,
		 ConstMemoryDesc *ret_str)
{
    if (ret_str != NULL)
	*ret_str = str;

    if (str.getMemory () == NULL || str.getLength () == 0)
	return false;

    if (suffix.getMemory () == NULL || suffix.getLength () == 0)
	return true;

    if (str.getLength () < suffix.getLength ())
	return false;

    for (Size i = 0; i < suffix.getLength (); i++) {
	if (str.getMemory () [str.getLength () - i - 1] !=
	    suffix.getMemory () [suffix.getLength () - i - 1])
	{
	    return false;
	}
    }

    if (ret_str != NULL)
	*ret_str = ConstMemoryDesc (str.getMemory (), str.getLength () - suffix.getLength ());

    return true;
}

bool
stringHasSuffix (const char *str,
		 const char *suffix,
		 ConstMemoryDesc *ret_str)
{
    if (ret_str != NULL)
	*ret_str = str;

    if (str == NULL)
	return false;

    if (suffix == NULL)
	return true;

    return stringHasSuffix (ConstMemoryDesc (str, countStrLength (str)),
			    ConstMemoryDesc (suffix, countStrLength (suffix)),
			    ret_str);
}

#if 0
DEPRECATED

ComparisonResult
compareByteArrays (const unsigned char *str1,
		   unsigned long        str1_len,
		   const unsigned char *str2,
		   unsigned long        str2_len)
{
    if ((str1 == NULL && str1_len > 0) ||
	(str2 == NULL && str2_len > 0))
    {
	abortIfReached ();
    }

    unsigned long pos = 0;
    for (;;) {
	if (pos >= str1_len) {
	    if (pos >= str2_len)
		return ComparisonEqual;

	    return ComparisonLesser;
	}

	if (pos >= str2_len)
	    return ComparisonGreater;

	if (str1 [pos] > str2 [pos])
	    return ComparisonGreater;
	else
	if (str1 [pos] < str2 [pos])
	    return ComparisonLesser;

	pos ++;
    }

    /* Unreachable */
    abortIfReached ();
    return ComparisonEqual;
}

ComparisonResult
compareByteArrayToString (const unsigned char *str1,
			  unsigned long        str1_len,
			  const char          *str2)
{
    unsigned long pos = 0;
    for (;;) {
	if (pos >= str1_len) {
	    if (str2 [pos] == 0)
		return ComparisonEqual;

	    return ComparisonLesser;
	}

	if (str2 [pos] == 0)
	    return ComparisonGreater;

	if (str1 [pos] > (unsigned char) str2 [pos])
	    return ComparisonGreater;
	else
	if (str1 [pos] < (unsigned char) str2 [pos])
	    return ComparisonLesser;

	pos ++;
    }

    /* Unreachable */
    return ComparisonEqual;
}
#endif

unsigned long
strToUlong (const char *str)
{
    /* TODO Use a locale-indepentent function for conversion. */
    return ::strtoul (str, NULL, 0);
}

long
strToLong (const char *str)
{
    /* TODO Use a locale-indepentent function for conversion. */
    return ::strtol (str, NULL, 0);
}

double
strToDouble (const char *str)
{
    /* TODO Use a locale-indepentent function for conversion. */
    return ::strtod (str, NULL);
}

/* Assuming that all these characters are one-byte. Pretty lame. */
static const char digit_codes [16] =
	{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	  'A', 'B', 'C', 'D', 'E', 'F' };
static const char decimal_point = '.';
static const char minus_sign = '-';

char numToDigit (unsigned n)
{
    if (n >= 16) {
	abortIfReached ();
	return '?';
    }

    return digit_codes [n];
}

static bool ullongToStr_internal (unsigned long long ull,
				  char          *str,
				  unsigned long  len,
				  unsigned long *offs,
				  unsigned long  base)
{
    if (len == 0 &&
	str != NULL)
    {
	if (offs != NULL)
	    *offs = 0;
	return false;
    }

    unsigned long i = 1;
    unsigned long long div = 1;
    for (;;) {
	if (ull / div < base)
	    break;
	i ++;
	div *= base;
    }

    if (str == NULL) {
	if (offs != NULL)
	    *offs = i;
	return true;
    }

    unsigned long long ull_next;
    unsigned long digit,
		  pos = 0;
    for (; i > 0; i--) {
	if (pos >= len - 1) {
	    str [len - 1] = 0;
	    if (offs != NULL)
		*offs = pos;
	    return false;
	}

	ull_next = ull % div;
	digit = ull / div;
	str [pos] = numToDigit (digit);
	pos ++;
	ull = ull_next;
	div /= base;
    }

    str [pos] = 0;
    if (offs != NULL)
	*offs = pos;

    return true;
}

bool ullongToHexStr (unsigned long long ull,
		     char          *str,
		     unsigned long  len,
		     unsigned long *offs)
{
    return ullongToStr_internal (ull, str, len, offs, 16);
}

bool ullongToStr (unsigned long long ull,
		  char          *str,
		  unsigned long  len,
		  unsigned long *offs)
{
    return ullongToStr_internal (ull, str, len, offs, 10);
}

bool ulongToStr (unsigned long  ul,
		 char          *str,
		 unsigned long  len,
		 unsigned long *offs)
{
    return ullongToStr ((unsigned long long) ul, str, len, offs);
}

bool llongToStr (long long      ll,
		 char          *str,
		 unsigned long  len,
		 unsigned long *offs)
{
    if (len == 0 &&
	str != NULL)
    {
	if (offs != NULL)
	    *offs = 0;
	return false;
    }

    if (ll < 0) {
	if (str == NULL) {
	    /* FIXME It is possible that -ll will not be representable
	     * as an 'unsigned long long'. */
	    ullongToStr ((unsigned long long) -ll, NULL, 0, offs);
	    if (offs != NULL)
		(*offs) ++;
	    return true;
	}

	if (len == 1) {
	    str [0] = 0;
	    if (offs != NULL)
		*offs = 1;
	    return false;
	}

	str [0] = minus_sign;

	/* FIXME It is possible that -ll will not be representable
	 * as an 'unsigned long long'. */
	bool rv = ullongToStr ((unsigned long long) -ll,
			       str + 1, len - 1, offs);
	if (offs != NULL)
	    (*offs) ++;
	return rv;
    }

    return ullongToStr ((unsigned long) ll, str, len, offs);
}

bool longToStr (long           l,
		char          *str,
		unsigned long  len,
		unsigned long *offs)
{
    return llongToStr ((long long) l, str, len, offs);
}

/* 'prec' - precision, number of digits after the decimal point. */
bool doubleToStr (double         dbl,
		  char          *str,
		  unsigned long  len,
		  unsigned long *offs,
		  unsigned long  prec)
{
    /* TODO This code is very rough. I don't have enough experience
     * in dealing with floating point numbers. [DMS] */

    bool negative = false;

    if (len == 0 &&
	str != NULL)
    {
	if (offs != NULL)
	    *offs = 0;
	return false;
    }

    if (dbl < 0.0) {
	negative = true;

	if (str != NULL) {
	    if (len == 1) {
		str [0] = 0;
		if (offs != NULL)
		    *offs = 1;
		return false;
	    }

	    str [0] = minus_sign;
	}

	dbl = -dbl;
    }

    if (len == 0 &&
	str != NULL)
    {
	if (offs != NULL)
	    *offs = 0;
	return false;
    }

    unsigned long i = 1;
    double div = 1.0;
    for (;;) {
	if (dbl / div < 10.0)
	    break;

	i ++;
	div *= 10.0;

	if (i >= 16) {
	    /* TODO Uh... "large number" case.
	     * To be substituted with "e"-notation. */

	    const char *src_str = "(large number)";
	    unsigned long tocopy = countStrLength (src_str);

	    if (str != NULL) {
		if (tocopy > len)
		    tocopy = len;

		copyMemory (MemoryDesc (str, len), ConstMemoryDesc (src_str, tocopy));
	    }

	    if (offs != NULL)
		*offs = tocopy;

	    return false;
	}
    }

    if (str == NULL) {
	if (offs != NULL) {
	    *offs = i + prec;
	    if (negative)
		(*offs) ++;
	    if (prec > 0)
		(*offs) ++;
	}

	return true;
    }

    unsigned long pos;
    if (negative)
	pos = 1;
    else
	pos = 0;

    for (; i > 0; i--) {
	if (pos >= len - 1) {
	    str [len - 1] = 0;
	    if (offs != NULL)
		*offs = pos;
	    return false;
	}

	unsigned long digit;
	digit = (unsigned long) lround (trunc (dbl / div));
	str [pos] = numToDigit (digit);

	pos ++;
	dbl = fmod (dbl, div);
	div /= 10.0;
    }

    if (prec > 0) {
	str [pos] = decimal_point;
	pos ++;

	unsigned long j;
	for (j = 0; j < prec; j++) {
	    if (pos >= len - 1) {
		str [len - 1] = 0;
		if (offs != NULL)
		    *offs = pos;
		return false;
	    }

	    dbl *= 10.0;

	    unsigned long digit;
	    digit = (unsigned long) lround (trunc (dbl));
	    str [pos] = numToDigit (digit);

	    pos ++;
	    dbl = dbl - trunc (dbl);
	}
    }

    str [pos] = 0;
    if (offs != NULL)
	*offs = pos;

    return true;
}

#if 0
Banning printf* functions.

unsigned long
snprintf (char          *buf,
	  unsigned long  len,
	  const char    *format,
	  ...)
{
    int rv;
    va_list ap;

    va_start (ap, format);
    rv = g_vsnprintf (buf, len, format, ap);
    va_end (ap);

    if (rv < 0)
	rv = 0;

    return (unsigned long) rv;
}

unsigned long
vasprintf (char          *buf,
	   unsigned long  len,
	   const char    *format,
	   va_list        ap)
{
    int rv;
   
    rv = g_vsnprintf (buf, len, format, ap);
    if (rv < 0)
	rv = 0;

    return (unsigned long) rv;
}

/* TODO Convert these functions to return references to String objects. */
char* vmessage (const char *fmt, va_list orig_ap)
{
    int n;
    int size = 100;

    char *p;
    va_list ap;

    while (1) {
	p = new char [size];

	va_copy (ap, orig_ap);
	n = vsnprintf (p, size, fmt, ap);
	va_end (ap);

	if (n > -1 && n < size)
	    return p;

	if (n > -1)
	    size = n + 1;
	else
	    size *= 2;

	delete p;
    }

    /* Unreachable */
    return NULL;
}

char* message (const char *fmt, ...)
{
    char *str;
    va_list ap;

    va_start (ap, fmt);
    str = vmessage (fmt, ap);
    va_end (ap);

    return str;
}

int formatLength (const char *fmt, ...) {
    va_list ap;
    unsigned retval;

    va_start (ap, fmt);
    retval = vformatLength (fmt, ap);
    va_end (ap);

    return retval;
}

int vformatLength (const char *fmt, va_list ap)
{
    int n;
    int size;
    int prvsize;
    char stub;

    size = 1;
    while (1) {
	n = vsnprintf (&stub, 1, fmt, ap);
	if (n > -1)
	    return n;		// glibc 2.1
	else {
	    prvsize = size;
	    size <<= 1; 	// glibc 2.0
	}

	if (size < prvsize)	// overflow
				// (will not occur thanks to the types)
	    break;
    }

    return -1;
}
#endif

#ifndef PLATFORM_WIN32
Ref<String>
errnoToString (int code)
{
    char buf [4096];

    unsigned long r;

    if ((r = _mycpp_strerror_r (code, buf, sizeof buf)) == 0) {
	return grab (new String (buf));
    } 

    errf->print ("STRERROR").
	  print (r).
	  pendl ();

    if (errno == EINVAL)
	return grab (new String ("Invalid error code"));
    else
    if (errno == ERANGE)
	return grab (new String ("(error message is too large)"));
    else
	return grab (new String ("(failed to get an error string)"));
}

void
printError (const char *str)
{
    errf->print (str)
	 .print (errnoToString (errno))
	 .pendl ();
}
#else
// Very similar to M::win32ErrorToString().
Ref<String>
win32ErrorToString (DWORD error)
{
    LPTSTR pBuffer = NULL;

    DWORD rv;
    rv = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		       FORMAT_MESSAGE_ALLOCATE_BUFFER |
		       FORMAT_MESSAGE_IGNORE_INSERTS,
		       NULL, 
		       error,
		       MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
//		       MAKELANGID (LANG_ENGLISH, SUBLANG_ENGLISH_US),
		       (LPTSTR)&pBuffer, 
		       0, 
		       NULL);

    if (rv == 0) {
	if (pBuffer != NULL)
	    LocalFree (pBuffer);

	return String::forPrintTask (Pt (error));
    }

//    Ref<String> ret_str = String::forData (pBuffer);
    Ref<String> ret_str = String::forPrintTask (Pt ((const char*) "(") (error) ((const char*) ") ") (pBuffer));

    LocalFree (pBuffer);
    return ret_str;
}
#endif

}

