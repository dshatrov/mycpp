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

#ifndef __MYCPP__FILE_STREAM_IN_H__
#define __MYCPP__FILE_STREAM_IN_H__

#include <mycpp/object.h>
#include <mycpp/stream_in.h>
#include <mycpp/file.h>

namespace MyCpp {

class FileStreamIn : public StreamIn,
		     public virtual Object
{
protected:
    Ref<File> file;

public:
  /* StreamIn interface */

    IOResult read (unsigned char *buf,
		   unsigned long  len,
		   unsigned long *nread)
	    throw (IOException,
		   InternalException);

  /* (End of StreamIn interface) */

    FileStreamIn (File *file);
};

}

#endif /* __MYCPP__FILE_STREAM_IN_H__ */

