#ifndef __MYCPP_FILE_SOURCE_H__
#define __MYCPP_FILE_SOURCE_H__

#include <mycpp/file.h>

/* NOTE This code is not debugged, and is not used
 * anywhere. It is currently stuck in the middle of the process
 * of API transformation from (former) MyCpp::Source to MyCpp::File
 * interface. Currently, I do not consider this class particularly useful.
 * A better page-caching mechanism (with "distributed locality") will
 * most likely replace this one (with "current-position locality").
 *
 * UPDATE 07.07.01
 * I drop support for this class. Scheduled for removal from MyCpp. */

namespace MyCpp {

/* Buffered file wrapper. */
class BufferedFile : public File,
		     public virtual Object
{
protected:
    Ref<File> file;
    unsigned long bufferLength;

    char* buffers [3];
    int   currentBuffer;

    unsigned long bufferPosition,
		  unibufferPosition;

    unsigned long validLen,
		  univalidLen;

    /* File offset of the beginning of the first buffer */
    unsigned long long filePosition;
    unsigned long      readPosition;

    /* 0  - normal
     * 1  - EOF
     * -1 - error */
    int fillBuffers   ();
    int rotateBuffersLeft  ();
    int rotateBuffersRight ();
    int seekForward   (unsigned long long n);
    int seekBackwards (unsigned long long n);

public:
    /* File interface */
    unsigned long read (char *buf,
			unsigned long len)
		 throw (IOException);

    void write (const char *buf,
		unsigned long len)
	 throw (IOException);

    void seek  (long long offset,
		int origin)
	 throw (IOException);

    long long tell ();

    void flush () throw IOException ();
    /* (End of File interface) */

    /*m The constructor.
     *
     * @file a file to wrap.
     * @bufferLength length of a single buffer segment. */
    FileSource (File          *file,
		unsigned long  bufferLength);

    ~FileSource ();
};

};

#endif /* __MYCPP_FILE_SOURCE_H__ */

