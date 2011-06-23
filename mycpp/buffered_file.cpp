#include <mycpp/buffered_file.h>
#include <mycpp/io.h>

/* NOTE See buffered_file.h */

namespace MyCpp {

#define MYDUMP								\
    errf->print ("*****************************  "			\
		 "\ncurrentBuffer:     ".print (currentBuffer).		\
	  print ("\nvalidLen:          ".print (validLen).		\
	  print ("\nunivalidLen:       ".print (univalidLen).		\
	  print ("\nbufferPosition:    ".print (bufferPosition).	\
	  print ("\nunibufferPosition: ".print (unibufferPosition).	\
	  pendl ();

int
BufferedFile::fillBuffers ()
{
    if (univalidLen != readPosition)
	if (file->seek ((long long) univalidLen - (long long) readPosition,
			0) != 0)
	{
	    return -1;
	}

    long lastRead;

    while (validLen < bufferLength) {
	lastRead = file->read (buffers [currentBuffer] + validLen,
			       bufferLength - validLen);
	if (lastRead == -1)
	    return -1;
	if (lastRead == 0)
	    return 1;

	readPosition += lastRead;

	validLen    += lastRead;
	univalidLen += lastRead;
    }

    int bufNum = currentBuffer + 1;
    unsigned long inBuffer;

    while (bufNum < 3) {
	inBuffer = 0;
	while (inBuffer < bufferLength) {
	    lastRead = file->read (buffers [bufNum] + inBuffer,
				   bufferLength - inBuffer);
	    if (lastRead == -1)
		return -1;
	    if (lastRead == 0)
		return 1;

	    readPosition += lastRead;

	    inBuffer    += lastRead;
	    univalidLen += lastRead;
	}

	bufNum ++;
    }

    return 0;
}

int
BufferedFile::rotateBuffersLeft ()
{
    if (currentBuffer == 0) {
	errf->print ("MyCpp.FileSource.rotateBuffersLeft: "
		     "wrong state").pendl ();
	return -1;
    }

    char *tmp = buffers [0];
    buffers [0] = buffers [1];
    buffers [1] = buffers [2];
    buffers [2] = tmp;

    univalidLen -= bufferLength;
    readPosition -= bufferLength;

    currentBuffer --;
    filePosition += bufferLength;
    unibufferPosition -= bufferLength;

    return fillBuffers ();
}

int
BufferedFile::rotateBuffersRight ()
{
    if (currentBuffer == 2) {
	errf->print ("MyCpp.FileSource.rotateBuffersRight: "
		     "wrong state"). pendl ();
	return -1;
    }

    char *tmp = buffers [2];
    buffers [2] = buffers [1];
    buffers [1] = buffers [0];
    buffers [0] = tmp;

    univalidLen += bufferLength;
    if (univalidLen > 3 * bufferLength)
	univalidLen = 3 * bufferLength;

    currentBuffer ++;
    filePosition -= bufferLength;
    unibufferPosition += bufferLength;

    unsigned long inBuffer;
    long lastRead;

    if (file->seek (- (long long) readPosition - (long long) bufferLength, 0))
	return -1;
    readPosition = 0;

    inBuffer = 0;
    while (inBuffer < bufferLength) {
	lastRead = file->read (buffers [0] + inBuffer,
			       bufferLength - inBuffer);
	if (lastRead == -1)
	    return -1;
	if (lastRead == 0)
	    return 1;

	readPosition += lastRead;

	inBuffer    += lastRead;
	univalidLen += lastRead;
    }

    return 0;
}


unsigned long
BufferedFile::read (char *buf,
		    unsigned long len)
    throw (IOException)
{
//    errf->print ("MyCpp.FileSource.read").pendl ();
//    MYDUMP

    if (bufferPosition >= validLen) {
	errf->print ("MyCpp.FileSource.read: EOF").pendl ();
	return 0;
    }

    int character = buffers [currentBuffer] [bufferPosition];

    bufferPosition ++;
    unibufferPosition ++;
    if (bufferPosition >= bufferLength) {
	bufferPosition = 0;
	if (currentBuffer == 1) {
	    currentBuffer = 2;
	    validLen = univalidLen - (bufferLength << 1);
	    errf->print ("MyCpp.FileSource.read: "
			 "calling rotateBuffersLeft").pendl ();
	    rotateBuffersLeft ();
	} else {
	    currentBuffer = 1;
	    if (univalidLen > bufferLength << 1)
		validLen = bufferLength;
	    else
		validLen = univalidLen - bufferLength;
	}
    }

    return character;
}

void
BufferedFile::write (const char *buf,
		     unsigned long len)
    throw (IOException)
{
    throw IOException (grab (new String (
	    "write() operation is not supported for BufferedFile objects")));
}

int
BufferedFile::seekForward (unsigned long long n)
{
    if (bufferPosition + n < bufferPosition)
	return -1;

    while (currentBuffer < 3 &&
	   bufferPosition + n > bufferLength)
    {
	n -= bufferLength - bufferPosition;
	unibufferPosition += bufferLength - bufferPosition;
	bufferPosition = 0;
	currentBuffer ++;
    }

    if (currentBuffer >= 3) {
	if (univalidLen != 3 * bufferLength)
	    return -1;

	/* FIXME Calculate form readPosition? */
	if (file->seek ((long long) n +
			(long long) unibufferPosition -
			(long long) readPosition -
			bufferLength,
			0))
	    return -1;

	filePosition += unibufferPosition + n - bufferLength;
	readPosition = 0;

	currentBuffer  = 0;
	bufferPosition = 0;
	unibufferPosition = 0;
	validLen    = 0;
	univalidLen = 0;
	fillBuffers ();
	if (univalidLen <= bufferLength)
	    return -1;

	if (univalidLen > bufferLength << 1)
	    validLen = bufferLength;
	else
	    validLen = univalidLen - bufferLength;

	currentBuffer  = 1;
	bufferPosition = 0;
	unibufferPosition = bufferLength;
    } else {
	if (unibufferPosition + n >= univalidLen)
	    /* FIXME Lost state */
	    return -1;

	bufferPosition += n;
	unibufferPosition += n;

	if (bufferPosition >= bufferLength) {
	    bufferPosition = 0;
	    if (currentBuffer == 1) {
		currentBuffer = 2;
		validLen = univalidLen - (bufferLength << 1);
		errf->print ("MyCpp.FileSource.seekForward: "
			     "calling rotateBuffersLeft").pendl ();
		rotateBuffersLeft ();
	    } else {
		currentBuffer = 1;
		if (univalidLen > bufferLength << 1)
		    validLen = bufferLength;
		else
		    validLen = univalidLen - bufferLength;
	    }
	}
    }

    return 0;
}

int
BufferedFile::seekBackwards (unsigned long long n)
{
    if (bufferPosition + n < bufferPosition)
	return -1;

    if (unibufferPosition > n) {
	unibufferPosition -= n;
	/* FIXME Avoid division */
	bufferPosition = unibufferPosition % bufferLength;
	if (unibufferPosition > bufferLength) {
	    currentBuffer = 1;
	    if (univalidLen >= bufferLength << 1)
		validLen = bufferLength;
	    else
		validLen = univalidLen - bufferLength;
	} else {
	    currentBuffer = 0;
	    if (univalidLen >= bufferLength)
		validLen = bufferLength;
	    else
		validLen = univalidLen;

	    if (filePosition >= bufferLength) {
		errf->print ("MyCpp.FileSource.seekBackwards: "
			     "calling rotateBuffersRight").pendl ();
		MYDUMP
		rotateBuffersRight ();
	    }
	}
    } else {
	/* TODO There is a special case when the pointer is within
	 * one buffer length from the start of the first buffer,
	 * one block of date can be preserved in this case. */

	if (filePosition + unibufferPosition < n)
	    /* Seeking beyond the start of the file */
	    return -1;

	/* TODO Seek bufferLength bytes more if possible */
	if (file->seek (- ((long long) n - (long long) unibufferPosition + 
			   readPosition),
			0))
	    return -1;

	filePosition -= n - unibufferPosition;
	readPosition = 0;

	currentBuffer  = 0;
	bufferPosition = 0;
	unibufferPosition = 0;
	validLen    = 0;
	univalidLen = 0;
	fillBuffers ();

	if (univalidLen > bufferLength)
	    validLen = bufferLength;
	else
	    validLen = univalidLen;

	currentBuffer  = 0;
	bufferPosition = 0;
	unibufferPosition = bufferLength;
    }

    return 0;
}

void
BufferedFile::seek (long long offset,
		    int       origin)
    throw (IOException)
{
//    errf->print ("MyCpp.FileSource.seek").pendl ();
//    MYDUMP

    if (n > 0)
	seekForward ((unsigned long long) n);

    if (n < 0)
	seekBackwards ((unsigned long long) -n);
}

/*
void
BufferedFile::rewind ()
{
    currentBuffer = 0;

    bufferPosition    = 0;
    unibufferPosition = 0;

    validLen    = 0;
    univalidLen = 0;

    filePosition = 0;
    readPosition = 0;

    file->seek (0, -1);

    fillBuffers ();
}
*/

long long
BufferdFile::tell ()
{
    return filePosition + unibufferPosition;
}

BufferedFile::BufferedFile (File          *file,
			    unsigned long  bufferLength)
{
    this->file = file;
    this->bufferLength = bufferLength;

    buffers [0] = new char [bufferLength];
    buffers [1] = new char [bufferLength];
    buffers [2] = new char [bufferLength];

    currentBuffer     = 0;

    bufferPosition    = 0;
    unibufferPosition = 0;

    validLen    = 0;
    univalidLen = 0;

    filePosition = 0;
    readPosition = 0;

    fillBuffers ();
}

BufferedFile::~BufferedFile ()
{
    delete[] buffers [0];
    delete[] buffers [1];
    delete[] buffers [2];
}

};

