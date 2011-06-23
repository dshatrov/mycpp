#include <mycpp/file_stream_in.h>
#include <mycpp/util.h>

namespace MyCpp {

IOResult
FileStreamIn::read (unsigned char *buf,
		    unsigned long  len,
		    unsigned long *nread)
    throw (IOException,
	   InternalException)
{
    return file->read (buf, len, nread);
}

FileStreamIn::FileStreamIn (File *file)
{
    if (file == NULL)
	abortIfReached ();

    this->file = file;
}

}

