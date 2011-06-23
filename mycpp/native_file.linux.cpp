/* For lseek64() */
#define _LARGEFILE64_SOURCE

#include <cstdio> // DEBUG

#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <mycpp/native_file.h>
#include <mycpp/util.h>
#include <mycpp/io.h>

#define DEBUG(a) ;

namespace MyCpp {

IOResult
NativeFile::read (MemoryDesc const &mem,
		  Size *nread)
    throw (IOException,
	   InternalException)
{
    if (nread != NULL)
	*nread = 0;

    if (mem.getMemory () == NULL ||
	mem.getLength () == 0)
    {
	DEBUG (
	    fprintf (stderr, "MyCpp.NativeFile.read: no destination\n");
	    fflush (stderr);
	)
	return IOResultNormal;
    }

    ssize_t ret;

    /* According to POSIX, if we pass a value larger than SSIZE_MAX
     * to read, then the result is implementation-defined. */
    Size len;
    if (mem.getLength () > SSIZE_MAX)
	len = SSIZE_MAX;
    else
	len = mem.getLength ();

    rw_lock.readerLock ();
    if (fd == -1) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    abortIf ((Size) (size_t) len != len);
    DEBUG (
	fprintf (stderr, "MyCpp.NativeFile.read: len: %lu\n", (unsigned long) len);
	fflush (stderr);
    )
    ret = ::read (fd, mem.getMemory (), (size_t) len);
    rw_lock.readerUnlock ();

    if (ret == -1) {
	if (errno == EINTR)
	    return IOResultNormal;

	throw IOException (errnoToString (errno));
    } else
    if (ret < 0) {
	throw IOException ();
    } else
    if (ret == 0) {
	return IOResultEof;
    }

    if ((Size) ret > len)
	throw InternalException (String::forData ("Unexpectedly large return value from read()"));

    if (nread != NULL)
	*nread = (Size) ret;

    return IOResultNormal;
}

IOResult
NativeFile::write (ConstMemoryDesc const &mem,
		   Size *nwritten)
    throw (IOException,
	   InternalException)
{
    if (nwritten != NULL)
	*nwritten = 0;

    if (mem.getMemory () == NULL ||
	mem.getLength () == 0)
    {
	return IOResultNormal;
    }

    ssize_t ret;

    /* According to POSIX, if we pass a value larger than SSIZE_MAX
     * to write, then the result is implementation-defined. */
    Size len;
    if (mem.getLength () > SSIZE_MAX)
	len = SSIZE_MAX;
    else
	len = mem.getLength ();

    rw_lock.readerLock ();
    if (fd == -1) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    abortIf ((Size) (size_t) len != len);
    ret = ::write (fd, mem.getMemory (), (size_t) len);
    rw_lock.readerUnlock ();

    if (ret == -1) {
	if (errno == EINTR)
	    return IOResultNormal;

	if (errno == EPIPE)
	    return IOResultEof;

      /* DEBUG */
	errf->print ("MyCpp.NativeFile.write_raw: ret == -1").pendl ();
      /* (DEBUG) */

	throw IOException (errnoToString (errno));
    } else
    if (ret < 0) {
      /* DEBUG */
	errf->print ("MyCpp.NativeFile.write_raw: ret < 0").pendl ();
      /* (DEBUG) */

	throw IOException ();
    }

    if ((Size) ret > len)
	throw InternalException (String::forData ("Unexpectedly large return value from write()"));

    if (nwritten != NULL)
	*nwritten = (Size) ret;

    return IOResultNormal;
}

void
NativeFile::seek (Int64      offset,
		  SeekOrigin origin)
    throw (IOException,
	   InternalException)
{
    int whence = 0;

    switch (origin) {
    case SeekOrigin::SeekBeg:
	whence = SEEK_SET;
	break;
    case SeekOrigin::SeekCur:
	whence = SEEK_CUR;
	break;
    case SeekOrigin::SeekEnd:
	whence = SEEK_END;
	break;
    }

    rw_lock.readerLock ();
    if (fd == -1) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    /* NOTE: lseek64() seems to be glibc-specific. */
    if (lseek64 (fd, (off64_t) offset, whence) == (off64_t) -1) {
	rw_lock.readerUnlock ();
	throw IOException (errnoToString (errno));
    }
    rw_lock.readerUnlock ();
}

Uint64
NativeFile::tell ()
    throw (InternalException)
{
    off_t ret;

    rw_lock.readerLock ();
    if (fd == -1) {
	rw_lock.readerUnlock ();
	throw InternalException ();
    }

    /* I couldn't find a suitable syscall to read file offset
     * as an unsigned 64-bit number. Hence, there is no way
     * of reading really large file offsets. We are limited
     * by Int64 type, not Uint64. */
    ret = lseek (fd, 0, SEEK_CUR);
    rw_lock.readerUnlock ();

    if (ret == (off_t) -1)
	throw InternalException (errnoToString (errno));

    if (ret < 0)
	throw InternalException (
		String::forPrintTask (
		    (Pr ("Unexpected return value from lseek(): "))
		    (Pr ((long long) ret))));

    Uint64 ret64 = (Uint64) ret;

    /* Uint64 must be able to hold any value of type 'off_t',
     * or at least the file position should not be too large if one
     * wants to get it with MyCpp.NativeFile.tell() . */
    if ((off_t) ret64 != ret)
	throw InternalException (
		String::forData ("Integer overflow"));

    return ret64;
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
    do {
	int ret;

	rw_lock.readerLock ();
	if (fd == -1) {
	    rw_lock.readerUnlock ();
	    throw InternalException ();
	}

	ret = fsync (fd);
	rw_lock.readerUnlock ();

	if (ret == -1) {
	    if (errno == EINTR)
		continue;

	    throw IOException (errnoToString (errno));
	}

	if (ret != 0) {
	    throw IOException (
		    String::forPrintTask (
			(Pr ("Unexpected return value from fsync(): "))
			(Pr ((long) ret))));
	}

	break;
    } while (true);
}

void
NativeFile::close (bool /* flush_data */)
    throw (IOException,
	   InternalException)
{
    rw_lock.writerLock ();
    if (fd == -1) {
	rw_lock.writerUnlock ();
	// NOTE: Multiple invocations of close() are allowed.
	// Multiple threads should be able to close() the file at will.
	return;
    }

    int tmp_fd = fd;
    fd = -1;
    rw_lock.writerUnlock ();

    for (;;) {
	int res = ::close (tmp_fd);

	// FIXME res == -1 (+ unexpected != 0)
	if (res != 0) {
	    if (errno == EINTR)
		continue;

	    throw IOException ();
	}

	break;
    }
}

NativeFile::NativeFile (const char    *filename,
			unsigned long  openFlags,
			AccessMode     accessMode)
    throw (IOException,
	   InternalException)
{
    should_close = true;

    int flags = O_RDONLY;

    switch (accessMode) {
    case AccessMode::ReadOnly:
	flags = O_RDONLY;
	break;
    case AccessMode::WriteOnly:
	flags = O_WRONLY;
	break;
    case AccessMode::ReadWrite:
	flags = O_RDWR;
	break;
    }

    if (openFlags & OpenFlags::Create)
	flags |= O_CREAT;
    /* TODO Specify behavior for "Truncate & O_RDONLY" combination. */
    if (openFlags & OpenFlags::Truncate)
	flags |= O_TRUNC;

    for (;;) {
	/* NOTE: man 2 open does not mention EINTR as a possible return
	 * value, while man 3 open _does_. This means that EINTR should
	 * be handled for all invocations of open() in MyCpp (and all
	 * over MyNC). */
	fd = open (filename,
		   /* O_DIRECT is not what I actually thought it to be. */
		   flags /*| O_DIRECT*/,
		   S_IRUSR | S_IWUSR);
	if (fd == -1) {
	    if (errno == EINTR)
		continue;

	    throw IOException (errnoToString (errno));
	}

	break;
    }
}

NativeFile::NativeFile (int fd,
			bool should_close)
{
    this->fd = fd;
    this->should_close = should_close;
}

NativeFile::~NativeFile ()
{
    if (fd != -1 &&
	should_close)
    {
	errf->print ("MyCpp.NativeFile.~(): WARNING: file has not been closed").pendl ();
	for (;;) {
	    int res = ::close (fd);
	    if (res == -1) {
		if (errno == EINTR)
		    continue;

		errf->print ("MyCpp.NativeFile.~(): WARNING: close() failed").pendl ();
	    } else
	    if (res != 0) {
		errf->print ("MyCpp.NativeFile.~(): WARNING: unexpected return value from close()").pendl ();
	    }

	    break;
	}
    }
}

}

