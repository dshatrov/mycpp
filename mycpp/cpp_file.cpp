#include <mycpp/util.h>

#include <mycpp/cpp_file.h>

// WARNING: Currently, this is a very rough implementation for testing purposes only.
// It doesn't handle errors. It doesn't really obey MyCpp::File semantics.
// It doesn't honour flush() and sync().

using namespace std;
using namespace MyCpp;

namespace MyCpp {

IOResult
CppFile::read (MemoryDesc const &mem,
	       Size *nread)
    throw (IOException,
	   InternalException)
{
    if (nread != NULL)
	*nread = 0;

    file.read ((char*) mem.getMemory (), mem.getLength ());
    file.seekp (file.tellg (), ios_base::beg);
    if (nread != NULL)
	*nread = file.gcount ();

    if (file.gcount () == 0 &&
	file.eof ())
    {
	return IOResultEof;
    }

    return IOResultNormal;
}

IOResult
CppFile::write (ConstMemoryDesc const &mem,
		Size *nwritten)
    throw (IOException,
	   InternalException)
{
    if (nwritten != NULL)
	*nwritten = 0;

    file.write ((const char*) mem.getMemory (), mem.getLength ());
    file.seekg (file.tellp (), ios_base::beg);
    if (nwritten != NULL)
	*nwritten = mem.getLength ();

    return IOResultNormal;
}

void
CppFile::seek (Int64      offset,
	       SeekOrigin origin)
    throw (IOException,
	   InternalException)
{
    ios_base::seekdir dir = ios_base::beg;
    if (origin == SeekOrigin::SeekBeg)
	dir = ios_base::beg;
    else
    if (origin == SeekOrigin::SeekCur)
	dir = ios_base::cur;
    else
    if (origin == SeekOrigin::SeekEnd)
	dir = ios_base::end;
    else
	abortIfReached ();

    file.seekp (offset, dir);
    file.seekg (offset, dir);
}

Uint64
CppFile::tell ()
     throw (InternalException)
{
    streampos pos = file.tellp ();
    if (pos != file.tellg ())
	throw InternalException ();

    return pos;
}

void
CppFile::flush ()
    throw (IOException,
	   InternalException)
{
    // No-op
}

void
CppFile::sync ()
    throw (IOException,
	   InternalException)
{
    // No-op
}

void
CppFile::close (bool /* flush_data */)
    throw (IOException,
	   InternalException)
{
    // No-op
}

CppFile::CppFile (const char *filename,
		  Int32       open_flags,
		  AccessMode  access_mode)
{
    abortIf (filename == NULL);

    ios_base::openmode mode_flags = ios_base::binary;

    if (access_mode == AccessMode::ReadOnly)
	mode_flags |= ios_base::out;
    else
    if (access_mode == AccessMode::WriteOnly)
	mode_flags |= ios_base::in;
    else
    if (access_mode == AccessMode::ReadWrite)
	mode_flags |= ios_base::in | ios_base::out;
    else
	abortIfReached ();

    // WARNING: OpenFlags::Create is ignored

    if (open_flags & OpenFlags::Truncate)
	mode_flags |= ios_base::trunc;

    file.open (filename, mode_flags);
}

CppFile::~CppFile ()
{
    file.close ();
}

}

