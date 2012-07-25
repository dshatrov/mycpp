/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2009-2010 Dmitry Shatrov
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


#ifndef __MYCPP__TYPES_H__
#define __MYCPP__TYPES_H__


#include <libmary/types.h>

#include <stdint.h>

#include <cassert>

#include <cstdlib> // for NULL
//#include <glib/gtypes.h> /* For glib's basic types (guint32 etc.) */
#include <glib.h>


namespace MyCpp {

typedef gint16 Int16;
typedef gint32 Int32;
typedef gint64 Int64;

typedef guint16 Uint16;
typedef guint32 Uint32;
typedef guint64 Uint64;

typedef uintptr_t UintPtr;

typedef unsigned char Byte;
typedef unsigned long Size;
typedef long          Offset;

#define MYCPP_SIZE_MIN ULONG_MIN
#define MYCPP_SIZE_MAX ULONG_MAX

#define MYCPP_UINT32_MIN G_MINUINT32
#define MYCPP_UINT32_MAX G_MAXUINT32

#define MYCPP_INT64_MIN G_MININT64
#define MYCPP_INT64_MAX G_MAXINT64

#define MYCPP_UINT64_MIN G_MINUINT64
#define MYCPP_UINT64_MAX G_MAXUINT64

class Bool
{
protected:
    bool value;

public:
    Bool& operator = (bool value)
    {
	this->value = value;
	return *this;
    }

    operator bool () const
    {
	return value;
    }

    Bool (bool value)
	: value (value)
    {
    }

    Bool ()
    {
	value = false;
    }
};

enum ComparisonResult
{
    ComparisonEqual = 0,
    ComparisonLesser,
    ComparisonGreater
};

class FuzzyResult
{
public:
    enum _FuzzyResult
    {
	Yes   = 1,
	No    = 0,
	Maybe = 2
    };

    _FuzzyResult value;

    operator _FuzzyResult () const
    {
	return value;
    }

    FuzzyResult (_FuzzyResult const &value)
    {
	this->value = value;
    }
};

enum IOResult {
    IOResultNormal = 0,
    IOResultAgain,
    IOResultEof
};

/* File-related enumerations */

    /*e File open mode. */
    class AccessMode
    {
    public:
	enum _AccessMode {
	    ReadOnly = 0, /*< Open the file for reading. */
	    WriteOnly,    /*< Open the file for writing. */
	    ReadWrite     /*< Open the file for both reading and writing. */
	} value; // FIXME private

	operator unsigned long () const
	{
	    return value;
	}

	AccessMode (_AccessMode value)
	    : value (value)
	{
	}

	AccessMode ()
	{
	}
    };

    /*e File open flags. */
    class OpenFlags
    {
    protected:
	Int32 value;

    public:
	enum _OpenFlags {
	    Create   = 0x1,	//< Create the file if it does not exist.
	    Truncate = 0x2	//< Truncate the file if it exists.
	/* This flag has problems with NFS. Is it really necessary?
	    NewOnly  = 0x4	//< Throw an exception if the file already exists.
	*/
	};

	operator Int32 () const
	{
	    return value;
	}

	OpenFlags (Int32 value)
	    : value (value)
	{
	}

	OpenFlags ()
	    : value (0)
	{
	}
    };

    /*e File seek origin. */
    class SeekOrigin
    {
    public:
	enum _SeekOrigin {
	    SeekBeg = 0,	//< Seek from the begining of a file.
	    SeekCur,	//< Seek from the current position in a file.
	    SeekEnd		//< Seek from the end of a file.
	} value;

	operator unsigned long () const
	{
	    return value;
	}

	SeekOrigin (_SeekOrigin value)
	    : value (value)
	{
	}

	SeekOrigin ()
	{
	}
    };

/* (End of file-related enumerations) */

using M::VoidFunction;

// TODO Get rid of this
typedef void (*RefCallback) (void *data);

using M::EmptyBase;

}


#endif /* __MYCPP__TYPES_H__ */

