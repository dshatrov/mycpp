#ifndef __MYCPP_DIRECT_FILE_H__
#define __MYCPP_DIRECT_FILE_H__

#include <mycpp/file.h>
#include <mycpp/io_exception.h>

namespace MyCpp {

/* 07.07.01
 * This class is cool, indeed, but it duplicates functionality of
 * NatveFile, which is not good. This class should be removed
 * from MyCpp, I think. */

// 09.08.04 This class uses standard C library calls,
// so there's some real value in it. Could be nice for portability, etc.
// TODO De-bitrotting, review, refactoring. Rename to CstdFile

/*c A direct <t>File</t> implementation, with all methods directly mapped
 * to filesystem calls.
 *
 * <t>DirectFile</t> explicitly does not use buffering for I/O operations,
 * so, in general, an additional buffering layer is desired when
 * using <t>DirectFile</t> objects.*/
class DirectFile : public File,
		   public virtual Object
{
protected:
    FILE *file;

    static const char* modeToMstring (AccessMode mode);

public:
    /* File interface */
    unsigned long read  (char          *buf,
			 unsigned long  len)
		  throw (IOException);

    unsigned long write (const char    *buf,
			 unsigned long  len)
		  throw (IOException);

    void seek  (long long  offset,
		SeekOrigin origin)
	 throw (IOException);

    long long tell () throw (IOException);

    void flush () throw (IOException);
    /* (End of File interface) */

    /*m The contructor.
     *
     * Opens a file for reading and writing.
     * The current file position is set to the beginning of the file.
     *
     * Throws an <t>IOException</t> if the file
     * cannot be opened for some reason.
     *
     * @filename A name of the file to be opened. */
    DirectFile (const char *filename,
		AccessMode mode = ReadOnly)
	 throw (IOException);

    /*m*/
    DirectFile (int fd,
		AccessMode mode = ReadOnly)
	 throw (IOException);

    ~DirectFile ();
};

};

#endif /* __MYCPP_DIRECT_FILE_H__ */

