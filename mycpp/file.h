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

#ifndef __MYCPP__FILE_H__
#define __MYCPP__FILE_H__

#include <mycpp/types.h>
#include <mycpp/string.h>
#include <mycpp/object.h>
#include <mycpp/io_exception.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

class PrintTask;

/*c Generic file abstraction. */
class File : public virtual Object
{
public:
    /*m Reads a chunk of data at the current position.
     *
     * If more that zero bytes was requested, then at least
     * one byte is guaranteed to be read.
     *
     * This method should be expected to throw an <t>IOException</t>.
     *
     * Returns the total number of bytes read into the buffer,
     * or 0 if there is no more data because
     * the end of the file has been reached. Also returns 0 if
     * zero bytes was requested to be read.
     *
     * @buf the buffer into which the data is read.
     * @len the maximum number of bytes to read. */
    virtual IOResult read (MemoryDesc const &mem,
			   Size *nread)
		    throw (IOException,
			   InternalException) = 0;

    /*m Writes a chunk of data at the current position.
     *
     * This method should be expected to throw an <t>IOException</t>.
     *
     * @len the number of bytes to write. */
    virtual IOResult write (ConstMemoryDesc const &mem,
			    Size *nwritten)
		     throw (IOException,
			    InternalException) = 0;

    /*m Changes current file position.
     *
     * Positioning beyond the end or before the beginning of the file
     * is not allowed.
     *
     * This method should be expected to throw an <t>IOException</t>.
     *
     * @offset an offset to seek by.
     * @origin 
     * <ul>
     * <li>-1 - seek from the beginning of the file.</li>
     * <li>0 - seek from the current position.</li>
     * <li>1 - seek from the end of the file (the offset should
     * be negative in this case). </li>
     * </ul> */
    virtual void seek (Int64      offset,
		       SeekOrigin origin)
		throw (IOException,
		       InternalException) = 0;

    /*m Returns current offset from the beginning of the file. */
    virtual Uint64 tell ()
		  throw (InternalException) = 0;

    /*m Flushes the file, forcing any buffered bytes to be written out.
     *
     * The <t>flush</t> method is expected to write out all pending data
     * <i>and</i> to flush the underlying transport/storage facility,
     * so that all chained <t>File</t> objects get flushed, when the topmost
     * <t>File</t> gets flushed.
     *
     * Note that flush() does not guarantee writing of data to the underlying
     * physical medium. It only guarantees that the data has been transfered
     * to the operating system. If you need a mechanism to ensure that
     * data has actually made its way to the medium (e.g. to implement
     * database transactions), then use the sync() method.
     *
     * This method should be expected to throw an <t>IOException</t>. */
    virtual void flush ()
		 throw (IOException,
			InternalException) = 0;

    /*m Forces writing to underlying physical storage. */
    virtual void sync ()
		 throw (IOException,
			InternalException) = 0;

    /* 07.08.05
     * Q: Why is there no close() method?
     * 
     * A: Closing a file only makes sense if there is an agent acting
     * on the "opposite side" of the file (e.g. on the other end
     * of a pipe). For example, closing does nothing for array-backed files
     * (ArrayFile class), and for regular files (NativeFile class) it
     * makes sense, because there's a kernel "on the other end".
     *
     * In general, actual closing should be expected to be
     * deferred until all current read/write/seek/flush operations complete.
     * This is because POSIX is certainly not ideal for multi-threaded
     * applications, and there's no way to fix races on file
     * descriptors in cases like simultaneous calls to read() and close()
     * without holding a mutex for the whole duration of read().
     * If close() gets called just before read(), than it is possible
     * that open() will be called from a different thread and
     * another file will be opened with the same descriptor, and thus
     * read() will end up reading from a wrong file.
     * This may be a problem when performing synchronous I/O.
     *
     * For asynchronous I/O this gets even worse, because in this case
     * Pollable::getFd() gives the user complete control of the
     * file descriptor, and thus it should be guaranteed that
     * the descriptor remains valid for the whole lifetime of
     * the File object. Imagine the same sample case as in the previous
     * paragraph, but with PollGroup::iteration() instead of read().
     * Since PollGroup::iteration() gets called by the user,
     * the File object does not have any control of it, and thus it
     * is not even possible to use a mutex to prevent closing
     * a descriptor that is just about to be used in a different
     * thread.
     *
     * Provided the above, I think that the best solution will be
     * to state that a file gets closed when the corresponding File object
     * gets destroyed.
     *
     * 08.05.30 Update: I think that it's better to leave close() method
     * in place and just document the shortcomings of POSIX-backed
     * implementations.
     *
     * 09.07.18 Update: I'll do it with POSIX anyway. See PosixFdContainer
     * class in the posix.h header. One _must_ call close() for the file
     * in order to be sure that all of the data has been successfully
     * transferred to lower levels.
     */

    /* (Old deprecated comment) Support for closing is not strictly required.
     * Also note that corresponding OS resources (like POSIX file descriptors)
     * may well be freed only when the File object is destroyed.
     */

    virtual void close (bool flush_data)
		 throw (IOException,
			InternalException) = 0;

    static Ref<File> createDefault (const char *filename,
				    OpenFlags   open_flags,
				    AccessMode  access_mode);

    static Ref<File> createDefault (ConstMemoryDesc const &filename,
				    OpenFlags  open_flags,
				    AccessMode access_mode);


  /* Non-virtual methods */

    void seekSet (Uint64 offset)
	   throw (IOException,
		  InternalException);

    void seekForward (Uint64 offset)
	       throw (IOException,
		      InternalException);

    void seekBackwards (Uint64 offset)
		 throw (IOException,
			InternalException);

    /*m Writes an exact number of bytes at the current position.
     *
     * This method operates by calling <c>write</c> method in
     * a loop until the requested portion of data gets fully
     * written to the file.
     *
     * [CORRECT THIS PARAGRAPH]
     * <c>fullWrite</c> does not support asynchronous files
     * (<t>AsyncFile</t> interface) in asynchoronous mode.
     * The behavior is undefined in this case.
     *
     * Differs from write() in that it never returns
     * less than 'len' bytes written and IOResultNormal
     * at the same time, except for cases when no more
     * data can be written.
     *
     * This method should be expected to throw an <t>IOException</t>. */
    IOResult fullWrite (ConstMemoryDesc const &mem,
			Size *bwritten)
		 throw (IOException,
			InternalException);

    /* Differs from read() in that it never returns
     * less than 'len' bytes read and IOResultNormal
     * at the same time, except for cases when there is
     * no more data to read. */
    IOResult fullRead (MemoryDesc const &mem,
		       Size *bread)
		throw (IOException,
		       InternalException);

    /*m Reads an integer at the current file position.
     *
     * After execution of this method the file position is
     * set just after an integer read. If there was no valid
     * integer, the position is not altered.
     *
     * This method should be expected to throw an <t>IOException</t>.
     *
     * Returns 0 on success.<br/>
     * Returns -1, if there is no valid integer at the current position.
     *
     * @result a placeholder for the result, may be <i>NULL</i>. */
    int readInt (int *result)
	  throw (IOException,
		 InternalException);

    /*m Reads a doble at the current file position.
     *
     * After execution of this method the file position is
     * set just after a double read. If there was no valid
     * double, the position is not altered.
     *
     * This method should be expected to throw an <t>IOException</t>.
     *
     * Returns 0 on success.<br/>
     * Returns -1, if there is no valid double at the current position.
     *
     * @result a placeholder for the result, may be <i>NULL</i>. */
    int readDouble (double *result)
	     throw (IOException,
		    InternalException);

    /*m FIXME Document this */
    Size readLine (MemoryDesc const &mem)
	    throw (IOException,
		   InternalException);

    /* "out" methods are backed by fullWrite() and throw exceptions.
     *
     * "print" methods never fail. They are intended for writing
     * non-critical messages to the streams nobody cares of
     * most of the time. */

    File& out (File *file) throw (IOException, InternalException);

    File& out (const String *str) throw (IOException, InternalException);
    mt_throws File& out (Ref<String> const &str) { return out ((String*) str); }

    File& out (const char *str) throw (IOException, InternalException);

    File& out (const Byte *str) throw (IOException, InternalException);

/* DEPRECATED
    File& out (const char    *buf,
	       unsigned long  len) throw (IOException, InternalException);
 */

    File& out (ConstMemoryDesc const &mdesc) throw (IOException, InternalException);

//    File& out (void const *ptr) throw (IOException, InternalException);

//    File& out (Uint32 ui23)            throw (IOException, InternalException);
//    File& out (Uint64 ui64)            throw (IOException, InternalException);
//    File& out (Int32 i32)              throw (IOException, InternalException);
//    File& out (Int64 i64)              throw (IOException, InternalException);
    File& out (unsigned long long ull) throw (IOException, InternalException);
    File& out (unsigned long ul)       throw (IOException, InternalException);
    File& out (unsigned u)             throw (IOException, InternalException);
    File& out (long long ll)           throw (IOException, InternalException);
    File& out (long l)                 throw (IOException, InternalException);
    File& out (int i)                  throw (IOException, InternalException);
    File& out (double dbl)             throw (IOException, InternalException);

    File& outHex (unsigned long long ull) throw (IOException, InternalException);

    File& out (const PrintTask &pt) throw (IOException, InternalException);

    File& oflush () throw (IOException, InternalException);
    File& oendl ()  throw (IOException, InternalException);

    static unsigned long nout (const String *str);
    static unsigned long nout (Ref<String> const &str) { return nout ((String*) str); }
    static unsigned long nout (const char *str);
    static unsigned long nout (const Byte *str);
/* DEPRECATED
    static unsigned long nout (const char    *buf,
			       unsigned long  len);
 */
    static unsigned long nout (ConstMemoryDesc const &mdesc);
//    static unsigned long nout (void const *ptr);
//    static unsigned long nout (Uint32 ui32);
//    static unsigned long nout (Uint64 ui64);
//    static unsigned long nout (Int32 i32);
//    static unsigned long nout (Int64 i64);
    static unsigned long nout (unsigned long long ull);
    static unsigned long nout (unsigned long ul);
    static unsigned long nout (unsigned u);
    static unsigned long nout (long long ll);
    static unsigned long nout (long l);
    static unsigned long nout (int i);
    static unsigned long nout (double dbl);

    static unsigned long noutHex (unsigned long long ull);

    File& print (File *file);

    File& print (const String *str);
    File& print (Ref<String> const &str) { return print ((String*) str); }

    /* str - 0-terminated string. */
    File& print (const char *str);

    File& print (const Byte *str);

#if 0
DEPRECATED
    /* buf - arbitrary array of character data,
     * not necessarily 0-terminated. */
    File& print (const char    *buf,
		 unsigned long  len);
#endif

    File& print (ConstMemoryDesc const &mdesc);

//    File& print (void const *ptr);

//    File& print (Uint32 ui32);
//    File& print (Uint64 ui64);
//    File& print (Int32 i32);
//    File& print (Int64 i64);
    File& print (unsigned long long ull);
    File& print (unsigned long ul);
    File& print (unsigned u);
    File& print (long long ll);
    File& print (long l);
    File& print (int i);
    File& print (double dbl);

    File& printHex (unsigned long long ull);

    File& print (const PrintTask &pt);

    File& pflush ();
    File& pendl ();
};

}

#endif /* __MYCPP__FILE_H__ */

