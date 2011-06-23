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

#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/array_file.h>

#define DEBUG(a)

/* NOTE that this code is very similar to that in direct_array_file.c.
 * If you make a fix here, then take a look at that file too. */

namespace MyCpp {

IOResult
ArrayFile::read (MemoryDesc const &mem,
		 Size *nread)
    throw (IOException,
	   InternalException)
{
    DEBUG (
	errf->print ("MyCpp.ArrayFile.read: " )
	     .print (mem.getLength ())
	     .print ("\n")
	     .print ("MyCpp.ArrayFile.read: this = ")
	     .print ((unsigned long) this)
	     .pendl ();
    ) /* (DEBUG) */

    /* For more cleanness. */
    if (nread != NULL)
	*nread = 0;

    if (array.isNull ()) {
	DEBUG (
	    errf->print ("MyCpp.ArrayFile.read: throwing IOException")
		 .pendl ();
	) /* (DEBUG) */

	throw InternalException (String::forData ("Array not set"));
    }

    if (position >= array->getCurrentSize ()) {
	DEBUG (
	    errf->print ("MyCpp.ArrayFile.read: returning zero, "
			 "position = ")
		 .print (position)
		 .print (", size = ")
		 .print (array->getCurrentSize ())
		 .pendl ();
	) /* (DEBUG) */

	return IOResultEof;
    }

    unsigned long lim;

    lim = array->getCurrentSize () - position;
    if (lim > mem.getLength ())
	lim = mem.getLength ();

    unsigned long i;
    for (i = 0; i < lim; i++) {
	mem.getMemory () [i] = array->element (position);
	position ++;
    }

    DEBUG (
	errf->print ("MyCpp.ArrayFile.read: returning ")
	     .print (lim)
	     .pendl ();
    ) /* (DEBUG) */

    if (nread != NULL)
	*nread = lim;

    return IOResultNormal;
}

IOResult
ArrayFile::write (ConstMemoryDesc const &mem,
		  Size *nwritten)
    throw (IOException,
	   InternalException)
{
    DEBUG (
	errf->print ("MyCpp.ArrayFile.write: ")
	     .print (mem.getLength ())
	     .pendl ();
    ) /* (DEBUG) */

    /* For more cleanness. */
    if (nwritten != NULL)
	*nwritten = 0;

    if (!writable)
	throw IOException (String::forData ("File is not writable"));

    if (array.isNull ())
	throw InternalException (String::forData ("Array not set"));

    if (position >= array->getSizeLimit ())
	throw IOException (String::forData ("Array is too short"));

    unsigned long lim;

    lim = array->getSizeLimit () - position;
    if (lim > mem.getLength ())
	lim = mem.getLength ();

    unsigned long i;
    for (i = 0; i < lim; i++) {
	array->element (position) = mem.getMemory () [i];
	position ++;
    }

    if (nwritten != NULL)
	*nwritten = mem.getLength ();

    return IOResultNormal;
}

/* TODO Take into account that offset is of type Int64. */
void
ArrayFile::seek (Int64      offset,
		 SeekOrigin origin)
    throw (IOException,
	   InternalException)
{
    DEBUG (
	errf->print ("MyCpp.ArrayFile.seek: ")
	     .print (offset)
	     .print (", ")
	     .print ((unsigned long) origin)
	     .pendl ();
    ) /* (DEBUG) */

    if (array.isNull ())
	throw InternalException (String::forData ("File not opened"));

    if (origin == SeekOrigin::SeekBeg) {
	if (offset < 0)
	    throw IOException (
		    String::forData ("Seek beyond the beginning of the array"));

	if ((unsigned long long) offset >
	    (unsigned long long) array->getSizeLimit ())
	{
	    throw IOException (
		    String::forData ("Seek beyond the size limit of the array"));
	}

	position = (unsigned long) offset;
    } else
    if (origin == SeekOrigin::SeekCur) {
	if (offset > 0) {
	    if ((unsigned long long) offset >
		(unsigned long long) (array->getSizeLimit () - position))
	    {
		throw IOException (
			String::forData (
			    "Seek beyond the size limit of the array"));
	    }

	    position += (unsigned long) offset;
	} else
	if (offset < 0) {
	    if ((unsigned long long) -offset >
		(unsigned long long) position)
	    {
		throw IOException (
			String::forData (
			    "Seek beyond the beginning of the array"));
	    }

	    position -= (unsigned long) -offset;
	}
    } else
    if (origin == SeekOrigin::SeekEnd) {
	if (offset > 0) {
	    if ((unsigned long long) offset >
		(unsigned long long) (array->getSizeLimit () -
				      array->getCurrentSize ()))
	    {
		throw IOException (
			String::forData ("Seek beyond the end of the array"));
	    }

	    position = array->getCurrentSize () + (unsigned long) offset;
	} else
	if ((unsigned long long) -offset >
	    (unsigned long long) array->getCurrentSize ())
	{
	    throw IOException (
		    String::forData ("Seek beyond the beginning of the array"));
	} else
	    position = array->getCurrentSize () - (unsigned long) -offset;
    } else
	abortIfReached_str ("Illegal seek origin");
}

Uint64
ArrayFile::tell ()
    throw (InternalException)
{
    DEBUG (
	errf->print ("MyCpp.ArrayFile.tell")
	     .pendl ();
    ) /* (DEBUG) */

    /* FIXME This assumes that 'Uint64' can hold any 'unsigned long' value. */
    return (Uint64) position;
}

void
ArrayFile::flush ()
    throw (IOException,
	   InternalException)
{
    DEBUG (
	errf->print ("MyCpp.ArrayFile.flush")
	     .pendl ();
    ) /* (DEBUG) */

    /* No-op. */
}

void
ArrayFile::sync ()
    throw (IOException,
	   InternalException)
{
    DEBUG (
	errf->print ("MyCpp.ArrayFile.sync")
	     .pendl ();
    ) /* (DEBUG) */

    /* No-op. */
}

void
ArrayFile::close (bool /* flush_data */)
    throw (IOException,
	   InternalException)
{
    // No-op
}

ArrayFile::ArrayFile (Array<unsigned char> *array,
		      bool writable)
{
    this->array = array;
    this->writable = writable;

    position = 0;
}

}

