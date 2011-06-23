/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2009  Dmitry M. Shatrov
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

#ifndef __MYCPP__ARRAY_FILE_H__
#define __MYCPP__ARRAY_FILE_H__

#include <mycpp/file.h>
#include <mycpp/array.h>

namespace MyCpp {

/*c A wrapper that allows to access arrays as files. */
class ArrayFile : public File,
		  public virtual Object
{
protected:
    Ref< Array<unsigned char> > array;
    bool writable;

    /* FIXME This is not MT-safe! __should be__
     * 08.02.28 ?! Isn't this class supposed to be MT-unsafe? */
    unsigned long position;

public:
    /* File interface */

    IOResult read (MemoryDesc const &mem,
		   Size *nread)
	    throw (IOException,
		   InternalException);

    IOResult write (ConstMemoryDesc const &mem,
		    Size *nwritten)
	     throw (IOException,
		    InternalException);

    void seek (Int64      offset,
	       SeekOrigin origin)
	throw (IOException,
	       InternalException);

    Uint64 tell ()
	  throw (InternalException);

    void flush ()
	 throw (IOException,
		InternalException);

    void sync ()
	throw (IOException,
	       InternalException);

    void close (bool flush_data)
	 throw (IOException,
		InternalException);

    /* (End of File interface) */

    /*m The constructor.
     *
     * @array The array to wrap.
     * @writable Whether the contents of the array can be modified or not. */
    ArrayFile (Array<unsigned char> *array,
	       bool writable);
};

}

#endif /* __MYCPP__ARRAY_FILE_H__ */

