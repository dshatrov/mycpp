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

#ifndef __MYCPP__NATIVE_FILE_H__
#define __MYCPP__NATIVE_FILE_H__

#include <Windows.h>

#include <mycpp/file.h>
#include <mycpp/rw_lock.h>

namespace MyCpp {

class NativeFile : public File,
		   public virtual Object
{
protected:
  // mt_const

    Bool should_close;

  // (end mt_const)

  // mt_lock rw_lock

    HANDLE fd;

  // (end mt_lock)

    RwLock rw_lock;

public:
  // File interface

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

  // (End of File interface)

    NativeFile (const char    *filename,
		unsigned long  openFlags,
		AccessMode     accessMode)
	 throw (IOException,
		InternalException);

    NativeFile (HANDLE fd,
		bool should_close);

    ~NativeFile ();
};

}

#endif /* __MYCPP__NATIVE_FILE_H__ */

