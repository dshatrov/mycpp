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

#ifndef __MYCPP__CPP_FILE_H__
#define __MYCPP__CPP_FILE_H__

#include <fstream>
#include <mycpp/file.h>

namespace MyCpp {

class CppFile : public File,
		public virtual Object
{
protected:
    std::fstream file;

public:
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

    CppFile (const char *filename,
	     Int32       open_flags,
	     AccessMode  access_mode);

    ~CppFile ();
};

}

#endif /* __MYCPP__CPP_FILE_H__ */

