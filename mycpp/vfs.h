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

#ifndef __MYCPP__VFS_H__
#define __MYCPP__VFS_H__

#include <mycpp/object.h>
#include <mycpp/string.h>
#include <mycpp/file.h>
#include <mycpp/async_file.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

class Vfs : public virtual Object
{
public:
    enum FileType
    {
	FileType_BlockDevice,
	FileType_CharacterDevice,
	FileType_Fifo,
	FileType_RegularFile,
	FileType_Directory,
	FileType_SymbolicLink,
	FileType_Socket
    };

    class StatData : public SimplyReferenced
    {
    public:
	unsigned long long size;
	FileType file_type;
    };

    class Directory : public SimplyReferenced
    {
    public:
	virtual Ref<String> getNextEntry ()
				   throw (InternalException) = 0;

	virtual void rewind ()
		      throw (InternalException) = 0;
    };

    virtual Ref<StatData> stat (const char *name)
			 throw (InternalException) = 0;

    virtual Ref<Directory> openDirectory (const char *dirname)
				   throw (InternalException) = 0;

    virtual Ref<File> openFile (const char    *filename,
				unsigned long  open_flags,
				AccessMode     access_mode)
			 throw (InternalException) = 0;

    virtual Ref<AsyncFile> openAsyncFile (const char    *filename,
					  unsigned long  open_flags,
					  AccessMode     access_mode)
				   throw (InternalException) = 0;

    static Ref<Vfs> defaultLocalVfs (const char *root_path);
};

}

#endif /* __MYCPP__VFS_H__ */

