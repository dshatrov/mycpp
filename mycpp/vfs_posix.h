#ifndef __MYCPP__VFS_POSIX_H__
#define __MYCPP__VFS_POSIX_H__

#include <sys/types.h>
#include <dirent.h>

#include <mycpp/vfs.h>

namespace MyCpp {

class VfsPosix : public Vfs,
		 public virtual Object
{
protected:
    Ref<String> root_path;

public:
    class Directory : public Vfs::Directory
    {
    protected:
	DIR *dir;

    public:
	Ref<String> getNextEntry ()
			   throw (InternalException);

	void rewind ()
	      throw (InternalException);

	Directory (const char *dirname)
	    throw (InternalException);

	~Directory ();
    };

    virtual Ref<StatData> stat (const char *name)
			 throw (InternalException);

    Ref<Vfs::Directory> openDirectory (const char *dirname)
				throw (InternalException);

    Ref<File> openFile (const char    *filename,
			unsigned long  open_flags,
			AccessMode     access_mode)
		 throw (InternalException);

    Ref<AsyncFile> openAsyncFile (const char    *filename,
				  unsigned long  open_flags,
				  AccessMode     access_mode)
			   throw (InternalException);

    VfsPosix (const char *root_path);
};

}

#endif /* __MYCPP__VFS_POSIX_H__ */

