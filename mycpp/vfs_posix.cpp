#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

#include <mycpp/array_holder.h>
#include <mycpp/native_file.h>
#include <mycpp/native_async_file.h>
#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/vfs_posix.h>

namespace MyCpp {

/* NOTE: glib functions from the "File Utilities" section are not
 * thread safe and cannot be used here (as of glib-2.12.13). */

Ref<String>
VfsPosix::Directory::getNextEntry ()
    throw (InternalException)
{
    ArrayHolder<unsigned char> dirent_array (sizeof (struct dirent) + NAME_MAX + 1);
    struct dirent * const dirent = (struct dirent*) &dirent_array [0];
    struct dirent *retp;

    int rv = readdir_r (dir, dirent, &retp);
    if (rv != 0)
	throw InternalException (errnoToString (rv));

    if (retp == NULL)
	return NULL;

    if (retp != dirent)
	throw InternalException ();

    return grab (new String (dirent->d_name));
}

void
VfsPosix::Directory::rewind ()
    throw (InternalException)
{
    rewinddir (dir);
}

VfsPosix::Directory::Directory (const char *dirname)
    throw (InternalException)
{
    dir = opendir (dirname);
    if (dir == NULL)
	throw InternalException (errnoToString (errno));
}

VfsPosix::Directory::~Directory ()
{
    for (;;) {
	int rv = closedir (dir);
	if (rv == -1) {
	    if (errno == EINTR)
		continue;

	    errf->print ("MyCpp.VfsPosix.Directory~: closedir() failed: ")
		 .print (errnoToString (errno))
		 .pendl ();
	} else
	if (rv != 0) {
	    errf->print ("MyCpp.VfsPosix.Directory~: unexpected return value from closedir(): ")
		 .print ((long) rv)
		 .pendl ();
	}

	break;
    }
}

Ref<Vfs::StatData>
VfsPosix::stat (const char *_name)
    throw (InternalException)
{
    Ref<String> name = String::forPrintTask ((Pr (root_path)) (Pr ("/")) (Pr (_name)));

    struct stat stat_buf;

    int rv = ::stat (name->getData (), &stat_buf);
    if (rv == -1) {
	throw InternalException (errnoToString (errno));
    } else
    if (rv != 0)
	throw InternalException ();

    Ref<StatData> stat_data = grab (new StatData);

    if (S_ISBLK (stat_buf.st_mode))
	stat_data->file_type = FileType_BlockDevice;
    else
    if (S_ISCHR (stat_buf.st_mode))
	stat_data->file_type = FileType_CharacterDevice;
    else
    if (S_ISDIR (stat_buf.st_mode))
	stat_data->file_type = FileType_Directory;
    else
    if (S_ISFIFO (stat_buf.st_mode))
	stat_data->file_type = FileType_Fifo;
    else
    if (S_ISREG (stat_buf.st_mode))
	stat_data->file_type = FileType_RegularFile;
    else
    if (S_ISLNK (stat_buf.st_mode))
	stat_data->file_type = FileType_SymbolicLink;
    else
    if (S_ISSOCK (stat_buf.st_mode))
	stat_data->file_type = FileType_Socket;
    else
	throw InternalException ();

    stat_data->size = (unsigned long long) stat_buf.st_size;

    return stat_data;
}

Ref<Vfs::Directory>
VfsPosix::openDirectory (const char *_dirname)
    throw (InternalException)
{
    Ref<String> dirname = String::forPrintTask ((Pr (root_path)) (Pr ("/")) (Pr (_dirname)));
    return grab (static_cast <Vfs::Directory*> (new Directory (dirname->getData ())));
}

Ref<File>
VfsPosix::openFile (const char    *_filename,
		    unsigned long  open_flags,
		    AccessMode     access_mode)
    throw (InternalException)
{
    Ref<String> filename = String::forPrintTask ((Pr (root_path)) (Pr ("/")) (Pr (_filename)));

    try {
	return grab (static_cast <File*> (new NativeFile (filename->getData (), open_flags, access_mode)));
    } catch (Exception &exc) {
	throw InternalException (String::nullString (), exc.clone ());
    }
}

Ref<AsyncFile>
VfsPosix::openAsyncFile (const char    *_filename,
			 unsigned long  open_flags,
			 AccessMode     access_mode)
    throw (InternalException)
{
    Ref<String> filename = String::forPrintTask ((Pr (root_path)) (Pr ("/")) (Pr (_filename)));

    try {
	return grab (static_cast <AsyncFile*> (new NativeAsyncFile (filename->getData (), open_flags, access_mode)));
    } catch (Exception &exc) {
	throw InternalException (String::nullString (), exc.clone ());
    }
}

VfsPosix::VfsPosix (const char *root_path)
    : root_path (grab (new String (root_path)))
{
}

}

