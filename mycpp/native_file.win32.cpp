#include <cstdio> // DEBUG

#include <Windows.h>

#include <mycpp/native_file.h>
#include <mycpp/util.h>
#include <mycpp/io.h>

#define DEBUG(a) ;

namespace MyCpp {

IOResult
NativeFile::read (MemoryDesc const &mem,
		  Size *ret_nread)
    throw (IOException,
	   InternalException)
{
    if (ret_nread != NULL)
	*ret_nread = 0;

    if (mem.getMemory () == NULL ||
	mem.getLength () == 0)
    {
	DEBUG (
	    fprintf (stderr, "MyCpp.NativeFile.read: no destination\n");
	    fflush (stderr);
	)
	return IOResultNormal;
    }

    rw_lock.readerLock ();
    if (fd == INVALID_HANDLE_VALUE) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    DWORD nread = 0;
    BOOL rv = ReadFile (fd, mem.getMemory (), mem.getLength (), &nread, NULL);
    rw_lock.readerUnlock ();

    if (rv == 0) {
	int error = GetLastError ();
	if (error == ERROR_HANDLE_EOF)
	    return IOResultEof;

	throw IOException (win32ErrorToString (error));
    }

    if (nread > mem.getLength ()) {
	errf->print ("MyCpp.NativeFile.read: ReadFile() reported too many bytes read").pendl ();
	throw InternalException ();
    }

    if (ret_nread != NULL)
	*ret_nread = (Size) nread;

    if (nread == 0)
	return IOResultEof;

    return IOResultNormal;
}

IOResult
NativeFile::write (ConstMemoryDesc const &mem,
		   Size *ret_nwritten)
    throw (IOException,
	   InternalException)
{
    if (ret_nwritten != NULL)
	*ret_nwritten = 0;

    if (mem.getMemory () == NULL ||
	mem.getLength () == 0)
    {
	return IOResultNormal;
    }

    rw_lock.readerLock ();
    if (fd == INVALID_HANDLE_VALUE) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    DWORD nwritten;
    BOOL rv = WriteFile (fd, mem.getMemory (), mem.getLength (), &nwritten, NULL);
    rw_lock.readerUnlock ();

    if (rv == 0)
	throw IOException (win32ErrorToString (GetLastError ()));

    if (nwritten > mem.getLength ()) {
	errf->print ("MyCpp.NativeFile.read: WriteFile() reported too many bytes written").pendl ();
	throw InternalException ();
    }

    if (ret_nwritten != NULL)
	*ret_nwritten = (Size) nwritten;

    return IOResultNormal;
}

void
NativeFile::seek (Int64      offset,
		  SeekOrigin origin)
    throw (IOException,
	   InternalException)
{
    DWORD whence = 0;

    switch (origin) {
    case SeekOrigin::SeekBeg:
	whence = FILE_BEGIN;
	break;
    case SeekOrigin::SeekCur:
	whence = FILE_CURRENT;
	break;
    case SeekOrigin::SeekEnd:
	whence = FILE_END;
	break;
    default:
	abortIfReached ();
    }

    rw_lock.readerLock ();
    if (fd == INVALID_HANDLE_VALUE) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    // FIXME Handle 64bit offsets properly
    DWORD ret = SetFilePointer (fd, (LONG) offset, NULL, whence);
    rw_lock.readerUnlock ();

    if (ret == INVALID_SET_FILE_POINTER) {
	int error = GetLastError ();
	if (error != NO_ERROR)
	    throw InternalException (win32ErrorToString (error));
    }
}

Uint64
NativeFile::tell ()
    throw (InternalException)
{
    DWORD ret;

    rw_lock.readerLock ();
    if (fd == INVALID_HANDLE_VALUE) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    // FIXME Handle 64bit offsets properly
    ret = SetFilePointer (fd, 0, NULL, FILE_CURRENT);
    rw_lock.readerUnlock ();

    if (ret == INVALID_SET_FILE_POINTER) {
	int error = GetLastError ();
	if (error != NO_ERROR)
	    throw InternalException (win32ErrorToString (error));
    }

    return (Uint64) ret;
}

void
NativeFile::flush ()
    throw (IOException,
	   InternalException)
{
    /* Nothing to do here. */
}

void
NativeFile::sync ()
    throw (IOException,
	   InternalException)
{
    BOOL ret;

    rw_lock.readerLock ();
    if (fd == INVALID_HANDLE_VALUE) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    ret = FlushFileBuffers (fd);
    rw_lock.readerUnlock ();

    if (ret == 0)
	throw IOException (win32ErrorToString (GetLastError ()));
}

void
NativeFile::close (bool /* flush_data */)
    throw (IOException,
	   InternalException)
{
    rw_lock.writerLock ();
    if (fd == INVALID_HANDLE_VALUE) {
	rw_lock.writerUnlock ();
	// NOTE: Multiple invocations of close() are allowed.
	// Multiple threads should be able to close() the file at will.
	return;
    }

    HANDLE tmp_fd = fd;
    fd = INVALID_HANDLE_VALUE;
    rw_lock.writerUnlock ();

    if (!CloseHandle (tmp_fd))
	throw IOException (win32ErrorToString (GetLastError ()));
}

NativeFile::NativeFile (const char    *filename,
			unsigned long  openFlags,
			AccessMode     accessMode)
    throw (IOException,
	   InternalException)
{
    should_close = true;

    DWORD access = 0;
    switch (accessMode) {
    case AccessMode::ReadOnly:
	access = GENERIC_READ;
	break;
    case AccessMode::WriteOnly:
	access = GENERIC_WRITE;
	break;
    case AccessMode::ReadWrite:
	access = GENERIC_READ | GENERIC_WRITE;
	break;
    default:
	abortIfReached ();
    }

    DWORD creation = OPEN_EXISTING;
    if (openFlags & OpenFlags::Truncate) {
	if (openFlags & OpenFlags::Create)
	    creation = CREATE_ALWAYS;
	else
	    creation = TRUNCATE_EXISTING;
    } else
    if (openFlags & OpenFlags::Create)
	creation = OPEN_ALWAYS;

    fd = CreateFile (filename,
		     access,
		     FILE_SHARE_READ | FILE_SHARE_WRITE,
		     NULL,
		     creation,
		     FILE_ATTRIBUTE_NORMAL,
		     NULL);
    if (fd == INVALID_HANDLE_VALUE)
	throw IOException (win32ErrorToString (GetLastError ()));
}

NativeFile::NativeFile (HANDLE fd,
			bool should_close)
{
    this->fd = fd;
    this->should_close = should_close;
}

NativeFile::~NativeFile ()
{
    if (fd != INVALID_HANDLE_VALUE &&
	should_close)
    {
	errf->print ("MyCpp.NativeFile.~(): WARNING: file has not been closed").pendl ();
	if (!CloseHandle (fd))
	    errf->print ("MyCpp.NativeFile.~(): WARNING: CloseHandle() failed").pendl ();
    }
}

}

