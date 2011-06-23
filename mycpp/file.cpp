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

// DEBUG
#include <stdio.h>

#include <cstring>

#include <mycpp/file.h>
#include <mycpp/util.h>
#include <mycpp/print_task.h>
#include <mycpp/io.h>

#define DEBUG(a) ;

namespace MyCpp {

void
File::seekSet (Uint64 offset)
    throw (IOException,
	   InternalException)
{
    bool first = true;

    while (offset > MYCPP_INT64_MAX) {
	if (first)
	    seek (MYCPP_INT64_MAX, SeekOrigin::SeekBeg);
	else
	    seek (MYCPP_INT64_MAX, SeekOrigin::SeekCur);

	offset -= MYCPP_INT64_MAX;
	first = false;
    }

    if (first)
	seek ((Int64) offset, SeekOrigin::SeekBeg);
    else {
	if (offset != 0)
	    seek ((Int64) offset, SeekOrigin::SeekCur);
    }
}

void
File::seekForward (Uint64 offset)
    throw (IOException,
	   InternalException)
{
    while (offset > MYCPP_INT64_MAX) {
	seek (MYCPP_INT64_MAX, SeekOrigin::SeekCur);

	offset -= MYCPP_INT64_MAX;
    }

    if (offset != 0)
	seek ((Int64) offset, SeekOrigin::SeekCur);
}

void
File::seekBackwards (Uint64 offset)
    throw (IOException,
	   InternalException)
{
    /* We need _both_ of the following to be true:
     *    -offset >= -MIN
     *     offset <=  MAX
     */

    /* Conjecture:
     *	 offset > -MIN
     *	 offset + MIN > 0
     *	 offset + MIN <= offset
     */
    while (offset + (Uint64) MYCPP_INT64_MIN <= offset) {
	seek (MYCPP_INT64_MIN, SeekOrigin::SeekCur);
	offset += (Uint64) MYCPP_INT64_MIN;
    }

    while (offset - (Uint64) MYCPP_INT64_MAX <= offset) {
	/* If we got here, then (-MYCPP_INT64_MAX) is representable
	 * as an Int64. */
	seek (-(Int64) MYCPP_INT64_MAX, SeekOrigin::SeekCur);
	offset -= (Uint64) MYCPP_INT64_MAX;
    }

    if (offset != 0) {
	/* Here, both (offset) and (-offset) are guaranteed to be
	 * representable as an Int64. */
	seek (-(Int64) offset, SeekOrigin::SeekCur);
    }
}

IOResult
File::fullWrite (ConstMemoryDesc const &mem,
		 Size *bwrittenRet)
	  throw (IOException,
		 InternalException)
{
    if (bwrittenRet != NULL)
	*bwrittenRet = 0;

    if (mem.getMemory () == NULL ||
	mem.getLength () == 0)
    {
	return IOResultNormal;
    }

    Size bwritten = 0;
    Size lastWritten;

    IOResult res = IOResultNormal;

    while (bwritten < mem.getLength ()) {
	res = write (mem.getRegion (bwritten, mem.getLength () - bwritten), &lastWritten);
	if (res != IOResultNormal)
	    break;

	bwritten += lastWritten;
    }

    if (bwrittenRet != NULL)
	*bwrittenRet = bwritten;

    if (res == IOResultEof &&
	bwritten != 0)
    {
	res = IOResultNormal;
    }

    return res;
}

IOResult
File::fullRead (MemoryDesc const &mem,
		Size *breadRet)
	 throw (IOException,
		InternalException)
{
    DEBUG (
	errf->print ("MyCpp.File.fullRead: ")
	     .print ("mem.getLength(): ").print (mem.getLength ()).pendl ();
    )

    if (breadRet != NULL)
	*breadRet = 0;

    if (mem.getMemory () == NULL ||
	mem.getLength () == 0)
    {
	return IOResultNormal;
    }

    Size bread = 0;
    Size lastRead;

    IOResult res = IOResultNormal;

    while (bread < mem.getLength ()) {
	DEBUG (
	    errf->print ("MyCpp.File.fullRead: ")
		 .print ("bread: ").print (bread).print (", ")
		 .print ("mem.getLength(): ").print (mem.getLength ()).pendl ();
	)
	res = read (mem.getRegion (bread, mem.getLength () - bread), &lastRead);
	DEBUG (
	    errf->print ("MyCpp.File.fullRead: ")
		 .print ("lastRead: ").print (lastRead).pendl ();
	)
	if (res != IOResultNormal)
	    break;

	bread += lastRead;
    }

    if (breadRet != NULL)
	*breadRet = bread;

    if (res == IOResultEof &&
	bread != 0)
    {
	res = IOResultNormal;
    }

    return res;
}

static inline void
skip_spaces (File *file)
      throw (IOException,
	     InternalException)
{
    unsigned char c;
    IOResult result;
    unsigned long bread;

    do {
	result = file->fullRead (MemoryDesc::forObject (c), &bread);
	if (result != IOResultNormal)
	    /* FIXME This is VERY rough. Result must be
	     * propagated to the caller. */
	    break;

	if (c != ' ' && c != '\t')
	    break;
    } while (1);

    if (result == IOResultNormal)
	file->seek (-1, SeekOrigin::SeekCur);
}

static inline bool
is_digit (int c)
{
    if (c <= '9' && c >= '0')
	return true;

    return false;
}

static inline int
to_number (int c)
{
    return c - '0';
}

int
File::readInt (int *retInt)
	throw (IOException,
	       InternalException)
{
    unsigned char c;
    bool has_one = false;
    bool negative = false;
    IOResult result;
    unsigned long nread;

    if (retInt != NULL)
	*retInt = 0;

    skip_spaces (this);

    do {
	result = fullRead (MemoryDesc::forObject (c), &nread);
	/* FIXME Rough! */
	if (result != IOResultNormal)
	    break;

	if (c == '-' && has_one == false) {
	    negative = true;
	    continue;
	}

	if (c == '+' && has_one == false)
	    continue;

	if (!is_digit (c))
	    break;

	has_one = true;
	if (retInt != NULL)
	    *retInt = *retInt * 10 + to_number (c);
    } while (1);

    if (result == IOResultNormal)
	seek (-1, SeekOrigin::SeekCur);

    if (has_one == false)
	return -1;

    if (negative) {
	if (retInt != NULL)
	    *retInt = - *retInt;
    }

    return 0;
}

int
File::readDouble (double *retDouble)
	   throw (IOException,
		  InternalException)
{
    unsigned char c;
    bool has_one = false;
    bool negative = false;
    double k = 0.1;
    IOResult result;
    unsigned long nread;

    if (retDouble != NULL)
	*retDouble = 0;

    skip_spaces (this);

    do {
	result = fullRead (MemoryDesc::forObject (c), &nread);
	/* FIXME Rough! */
	if (result != IOResultNormal)
	    break;

	if (c == '-' && has_one == false) {
	    negative = true;
	    continue;
	}

	if (c == '+' && has_one == false)
	    continue;

	if (!is_digit (c))
	    break;

	has_one = true;
	if (retDouble != NULL)
	    *retDouble = *retDouble * 10 + to_number (c);
    } while (1);

    if (has_one == false) {
	if (result == IOResultNormal)
	    seek (-1, SeekOrigin::SeekCur);

	return -1;
    }

    if (result == IOResultNormal) {
	if (c == '.') {
	    do {
		result = fullRead (MemoryDesc::forObject (c), &nread);
		/* FIXME Rough! */
		if (result != IOResultNormal)
		    break;

		if (!is_digit (c))
		    break;

		if (retDouble != NULL)
		    *retDouble += k * to_number (c);

		k *= 0.1;
	    } while (1);

	    if (result == IOResultNormal)
		seek (-1, SeekOrigin::SeekCur);
	} else {
	    seek (-1, SeekOrigin::SeekCur);
	}
    }

    if (negative) {
	if (retDouble != NULL)
	    *retDouble = - *retDouble;
    }

    return 0;
}

// TODO Right now, '\n' serves as a delimiter for strings.
// What about '\n\r' and/or '\r\n'?
Size
File::readLine (MemoryDesc const &mem)
	 throw (IOException,
		InternalException)
{
    Size bread,
	 last_read,
	 i;
    IOResult result;

    if (mem.getLength () == 0)
	return 0;

    Size len = mem.getLength ();
    // We return a zero-terminated string.
    len --;

    bread = 0;
    while (bread < len) {
	// The threshold is 512, this number is just lucky.
	// The reason for the threshold is that we do not want to read too much
	// beyond the end of the string. We'll seek back through unnecessary data anyway.
	result = fullRead (mem.getRegion (bread,
					  (len - bread) < 512 ? (len - bread) : 512),
			   &last_read);
	if (result != IOResultNormal) {
	    mem.getMemory () [bread] = 0;
	    return bread;
	}

	for (i = 0; i < last_read; i++) {
	    if (mem.getMemory () [bread + i] == '\n') {
		seek ((long long) (i + 1) - (long long) last_read,
		      SeekOrigin::SeekCur);
		bread = bread + i + 1;
		mem.getMemory () [bread] = 0;
		return bread;
	    }
	}

	bread += last_read;
    }

    mem.getMemory () [bread] = 0;
    return bread;
}

File&
File::out (File *file)
    throw (IOException,
	   InternalException)
{
    if (file != NULL) {
	unsigned char buf [4096];
	for (;;) {
	    unsigned long bread;
	    IOResult res;
	    res = file->fullRead (MemoryDesc::forObject (buf), &bread);
	    if (res != IOResultNormal)
		break;

	    out (ConstMemoryDesc::forObject (buf).getRegion (0, bread));
	}
    }

    return *this;
}

File&
File::out (const String *str)
    throw (IOException,
	   InternalException)
{
    if (str != NULL)
	out (str->getMemoryDesc ());

    return *this;
}

File&
File::out (const char *str)
    throw (IOException,
	   InternalException)
{
    if (str != NULL) {
	unsigned long len = countStrLength (str);
	out (ConstMemoryDesc (str, len));
    }

    return *this;
}

File&
File::out (const Byte *str)
    throw (IOException,
	   InternalException)
{
    if (str != NULL) {
	unsigned long len = countStrLength ((const char*) str);
	out (ConstMemoryDesc (str, len));
    }

    return *this;
}

#if 0
DEPRECATED
File&
File::out (const char    *buf,
	   unsigned long  len)
    throw (IOException,
	   InternalException)
{
    if (buf != NULL)
	fullWrite ((unsigned char*) buf, len, NULL);

    return *this;
}
#endif

File&
File::out (ConstMemoryDesc const &mdesc)
    throw (IOException,
	   InternalException)
{
    fullWrite (mdesc, NULL /* bwritten */);
    return *this;
}

#if 0
File&
File::out (void const *ptr)
    throw (IOException,
	   InternalException)
{
    return out ((Uint64) ptr);
}
#endif

#if 0
File&
File::out (Uint32 ui32)
    throw (IOException,
	   InternalException)
{
    return out ((unsigned long) ui32);
}
#endif

#if 0
File&
File::out (Uint64 ui64)
    throw (IOException,
	   InternalException)
{
    return out ((unsigned long long) ui64);
}
#endif

#if 0
File&
File::out (Int32 i32)
    throw (IOException,
	   InternalException)
{
    return out ((long) i32);
}

File&
File::out (Int64 i64)
    throw (IOException,
	   InternalException)
{
    unsigned char buf [32];
    unsigned long len;

    llongToStr ((long long) i64, (char*) buf, sizeof buf, &len);

    fullWrite (ConstMemoryDesc::forObject (buf).getRegion (0, len), NULL /* bwritten */);
    return *this;
}
#endif

File&
File::out (unsigned long long ull)
    throw (IOException,
	   InternalException)
{
    unsigned char buf [32];
    unsigned long len;

    ullongToStr (ull, (char*) buf, sizeof buf, &len);

    fullWrite (ConstMemoryDesc::forObject (buf).getRegion (0, len), NULL /* bwritten */);
    return *this;
}

File&
File::out (unsigned long ul)
    throw (IOException,
	   InternalException)
{
    unsigned char buf [32];
    unsigned long len;

    ulongToStr (ul, (char*) buf, sizeof buf, &len);

    fullWrite (ConstMemoryDesc::forObject (buf).getRegion (0, len), NULL /* bwritten */);
    return *this;
}

File&
File::out (unsigned u)
    throw (IOException,
	   InternalException)
{
    return out ((unsigned long) u);
}

File&
File::out (long long ll)
    throw (IOException,
	   InternalException)
{
    unsigned char buf [32];
    unsigned long len;

    llongToStr (ll, (char*) buf, sizeof buf, &len);

    fullWrite (ConstMemoryDesc::forObject (buf).getRegion (0, len), NULL /* bwritten */);
    return *this;
}

File&
File::out (long l)
    throw (IOException,
	   InternalException)
{
    unsigned char buf [32];
    unsigned long len;

    longToStr (l, (char*) buf, sizeof buf, &len);

    fullWrite (ConstMemoryDesc::forObject (buf).getRegion (0, len), NULL /* bwritten */);
    return *this;
}

File&
File::out (int i)
    throw (IOException,
	   InternalException)
{
    return out ((long) i);
}

File&
File::out (double dbl)
    throw (IOException,
	   InternalException)
{
    unsigned char buf [32];
    unsigned long len;

    /* TODO Hard-coded precision, provide a method for overriding. */
    if (!doubleToStr (dbl, (char*) buf, sizeof buf, &len, 3))
	errf->print ("MyCpp.File.out(double): doubleToStr() failed")
	     .pendl ();

    fullWrite (ConstMemoryDesc::forObject (buf).getRegion (0, len), NULL /* bwritten */);
    return *this;
}

File&
File::outHex (unsigned long long ull)
    throw (IOException,
	   InternalException)
{
    unsigned char buf [32];
    unsigned long len;

    ullongToHexStr (ull, (char*) buf, sizeof buf, &len);

    fullWrite (ConstMemoryDesc::forObject (buf).getRegion (0, len), NULL /* bwritten */);
    return *this;
}

File&
File::out (const PrintTask &pt)
    throw (IOException,
	   InternalException)
{
    pt.chainOut (this);
    return *this;
}

File&
File::oflush ()
    throw (IOException,
	   InternalException)
{
    flush ();
    return *this;
}

File&
File::oendl ()
    throw (IOException,
	   InternalException)
{
    out ("\n");
    oflush ();
    return *this;
}

unsigned long
File::nout (const String *str)
{
    if (str == NULL)
	return 0;

    return str->getLength ();
}

unsigned long
File::nout (const char *str)
{
    if (str == NULL)
	return 0;

    return countStrLength (str);
}

unsigned long
File::nout (const Byte *str)
{
    if (str == NULL)
	return 0;

    return countStrLength ((const char*) str);
}

#if 0
DEPRECATED
unsigned long
File::nout (const char *,
	    unsigned long len)
{
    return len;
}
#endif

unsigned long
File::nout (ConstMemoryDesc const &mdesc)
{
    return mdesc.getLength ();
}

#if 0
unsigned long
File::nout (void const *ptr)
{
    return nout ((Uint64) ptr);
}
#endif

#if 0
unsigned long
File::nout (Uint32 ui32)
{
    return nout ((unsigned long) ui32);
}
#endif

#if 0
unsigned long
File::nout (Uint64 ui64)
{
    return nout ((unsigned long long) ui64);
}
#endif

#if 0
unsigned long
File::nout (Int32 i32)
{
    return nout ((long) i32);
}

unsigned long
File::nout (Int64 i64)
{
    unsigned long len;
    llongToStr ((long long) i64, NULL, 0, &len);
    return len;
}
#endif

unsigned long
File::nout (unsigned long long ull)
{
    unsigned long len;
    ullongToStr (ull, NULL, 0, &len);
    return len;
}

unsigned long
File::nout (unsigned long ul)
{
    unsigned long len;
    ulongToStr (ul, NULL, 0, &len);
    return len;
}

unsigned long
File::nout (unsigned u)
{
    return nout ((unsigned  long) u);
}

unsigned long
File::nout (long long ll)
{
    unsigned long len;
    llongToStr (ll, NULL, 0, &len);
    return len;
}

unsigned long
File::nout (long l)
{
    unsigned long len;
    longToStr (l, NULL, 0, &len);
    return len;
}

unsigned long
File::nout (int i)
{
    return nout ((long) i);
}

unsigned long
File::nout (double dbl)
{
    unsigned long len;
    /* TODO Hard-coded precision, provide a method of overriding. */
    doubleToStr (dbl, NULL, 0, &len, 3);
    return len;
}

unsigned long
File::noutHex (unsigned long long ull)
{
    unsigned long len;
    ullongToHexStr (ull, NULL, 0, &len);
    return len;
}

File&
File::print (File *file)
{
    if (file != NULL) {
	try {
	    out (file);
	} catch (...) {
	}
    }

    return *this;
}

File&
File::print (const String *str)
{
    if (str != NULL)
	print (str->getMemoryDesc ());

    return *this;
}

File&
File::print (const char *str)
{
    if (str != NULL) {
	unsigned long len = countStrLength (str);
	print (ConstMemoryDesc (str, len));
    }

    return *this;
}

File&
File::print (const Byte *str)
{
    if (str != NULL) {
	unsigned long len = countStrLength ((const char*) str);
	print (ConstMemoryDesc (str, len));
    }

    return *this;
}

#if 0
DEPRECATED
File&
File::print (const char    *buf,
	     unsigned long  len)
{
    if (buf != NULL) {
	try {
	    out (buf, len);
	} catch (...) {
	}
    }

    return *this;
}
#endif

File&
File::print (ConstMemoryDesc const &mdesc)
{
    try {
	out (mdesc);
    } catch (...) {
    }

    return *this;
}

#if 0
File&
File::print (const void *ptr)
{
    try {
	out (ptr);
    } catch (...) {
    }

    return *this;
}
#endif

#if 0
File&
File::print (Uint32 ui32)
{
    try {
	out (ui32);
    } catch (...) {
    }

    return *this;
}
#endif

#if 0
File&
File::print (Uint64 ui64)
{
    try {
	out (ui64);
    } catch (...) {
    }

    return *this;
}
#endif

#if 0
File&
File::print (Int32 i32)
{
    try {
	out (i32);
    } catch (...) {
    }

    return *this;
}

File&
File::print (Int64 i64)
{
    try {
	out (i64);
    } catch (...) {
    }

    return *this;
}
#endif

File&
File::print (unsigned long long ull)
{
    try {
	out (ull);
    } catch (...) {
    }

    return *this;
}

File&
File::print (unsigned long ul)
{
    try {
	out (ul);
    } catch (...) {
    }

    return *this;
}

File&
File::print (unsigned u)
{
    try {
	out (u);
    } catch (...) {
    }

    return *this;
}

File&
File::print (long long ll)
{
    try {
	out (ll);
    } catch (...) {
    }

    return *this;
}

File&
File::print (long l)
{
    try {
	out (l);
    } catch (...) {
    }

    return *this;
}

File&
File::print (int i)
{
    try {
	out (i);
    } catch (...) {
    }

    return *this;
}

File&
File::print (double dbl)
{
    try {
	out (dbl);
    } catch (...) {
    }

    return *this;
}

File&
File::printHex (unsigned long long ull)
{
    try {
	outHex (ull);
    } catch (...) {
    }

    return *this;
}

File&
File::print (const PrintTask &pt)
{
    pt.chainPrint (this);
    return *this;
}

File&
File::pflush ()
{
    try {
	oflush ();
    } catch (...) {
    }

    return *this;
}

File&
File::pendl ()
{
    print ("\n");
    pflush ();

    return *this;
}

}

#ifndef PLATFORM_WIN32
#include <mycpp/native_file.h>
#else
//#include <mycpp/cpp_file.h>
#include <mycpp/native_file.h>
#endif

namespace MyCpp {

Ref<File>
File::createDefault (const char *filename,
		     OpenFlags open_flags,
		     AccessMode access_mode)
{
#ifndef PLATFORM_WIN32
    return grab (static_cast <File*> (new NativeFile (filename, open_flags, access_mode)));
#else
//    return grab (static_cast <File*> (new CppFile (filename, open_flags, access_mode)));
    return grab (static_cast <File*> (new NativeFile (filename, open_flags, access_mode)));
#endif
}

Ref<File>
File::createDefault (ConstMemoryDesc const &filename_,
		     OpenFlags  open_flags,
		     AccessMode access_mode)
{
    char filename [filename_.getLength() + 1];
    memcpy (filename, filename_.getMemory(), filename_.getLength());
    filename [filename_.getLength()] = 0;

    return grab (static_cast <File*> (new NativeFile (filename, open_flags, access_mode)));
}


}


