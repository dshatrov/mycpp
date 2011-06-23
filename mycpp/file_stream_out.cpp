#include <mycpp/file_stream_out.h>
#include <mycpp/util.h>

namespace MyCpp {

IOResult
FileStreamOut::write (const unsigned char *buf,
		      unsigned long  len,
		      unsigned long *nwritten)
    throw (IOException,
	   InternalException)
{
    return file->write (buf, len, nwritten);
}

FileStreamOut::FileStreamOut (File *file)
{
    if (file == NULL)
	abortIfReached ();

    this->file = file;
}

}

