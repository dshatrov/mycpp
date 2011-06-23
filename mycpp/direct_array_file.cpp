#include <mycpp/direct_array_file.h>
#include <mycpp/util.h>

/* NOTE that this code is very similar to that in array_file.cpp.
 * If you make a fix here, then take a look at that file too. */

namespace MyCpp {

IOResult
DirectArrayFile::read (MemoryDesc const &mem,
		       Size *nread)
    throw (IOException,
	   InternalException)
{
    if (nread != NULL)
	*nread = 0;

    if (array_mem.getMemory () == NULL)
	throw InternalException (String::forData ("Array not set"));

    if (pos >= array_mem.getLength ())
	return IOResultEof;

    Size lim;

    if (array_mem.getLength () - pos < mem.getLength ())
	lim = array_mem.getLength () - pos;
    else
	lim = mem.getLength ();

    copyMemory (mem, array_mem.getRegion (pos, lim));
    pos += lim;

    if (nread != NULL)
	*nread = lim;

    return IOResultNormal;
}

IOResult
DirectArrayFile::write (ConstMemoryDesc const &mem,
			Size *nwritten)
    throw (IOException,
	   InternalException)
{
    if (nwritten != NULL)
	*nwritten = 0;

    if (array_mem.getMemory () == NULL)
	throw InternalException (String::forData ("Array not set"));

    if (pos >= array_mem.getLength ())
	throw IOException (String::forData ("Array is too short"));

    Size lim;

    lim = array_mem.getLength () - pos;
    if (lim > mem.getLength ())
	lim = mem.getLength ();

    copyMemory (array_mem.getRegion (pos, array_mem.getLength () - pos),
		mem,
		lim);
    pos += lim;

    if (nwritten != NULL)
	*nwritten = lim;

    return IOResultNormal;
}

// TODO Revise all these abortIf()s for integer overflows.
// Ideally, there should be no abortIf()s at all. The function
// should handle all possible input values correctly.
void
DirectArrayFile::seek (Int64      offset,
		       SeekOrigin origin)
    throw (IOException,
	   InternalException)
{
    if (array_mem.getMemory () == NULL)
	throw InternalException (String::forData ("Array not set"));

    if (origin == SeekOrigin::SeekBeg) {
	if (offset < 0)
	    throw IOException (
		    String::forData (
			"Seek beyound the beginning of the array"));

	abortIf ((Int64) (Uint64) offset != offset);
	if ((Uint64) offset > (Uint64) array_mem.getLength ()) {
	    throw IOException (
		    String::forData ("Seek beyond size limit of the array"));
	}

	abortIf ((Int64) (Size) offset != offset);
	pos = (Size) offset;
    } else
    if (origin == SeekOrigin::SeekCur) {
	if (offset > 0) {
	    abortIf ((Int64) (Uint64) offset != offset);
	    if ((Uint64) offset > (Uint64) (array_mem.getLength () - pos)) {
		throw IOException (
			String::forData (
			    "Seek beyond size limit of the array"));
	    }

	    abortIf ((Int64) (Size) offset != offset);
	    pos += (Size) offset;
	} else
	if (offset < 0) {
	    abortIf ((Int64) (Uint64) (-offset) != -offset);
	    if ((Uint64) -offset > (Uint64) pos)
	    {
		throw IOException (
			String::forData (
			    "Seek beyond the beginning of the array"));
	    }

	    abortIf ((Int64) (Size) -offset != offset);
	    pos -= (Size) -offset;
	}
    } else
    if (origin == SeekOrigin::SeekEnd) {
	if (offset > 0) {
	    throw IOException (
		    String::forData ("Seek beyond size limit of the array"));
	}

	abortIf ((Int64) (Uint64) -offset != -offset);
	if ((Uint64) -offset > (Uint64) array_mem.getLength ()) {
	    throw IOException (
		    String::forData ("Seek beyond the beginning of the array"));
	}

	abortIf ((Int64) (Size) -offset != -offset);
	pos = array_mem.getLength () - (Size) -offset;
    } else
	abortIfReached_str ("Illegal seek origin");
}

Uint64
DirectArrayFile::tell ()
    throw (InternalException)
{
    /* FIXME This assumes that 'Uint64' can hold any 'unsigned long' value. */
    return (Uint64) pos;
}

void
DirectArrayFile::flush ()
    throw (IOException,
	   InternalException)
{
    /* No-op. */
}

void
DirectArrayFile::sync ()
    throw (IOException,
	   InternalException)
{
    /* No-op. */
}

void
DirectArrayFile::close (bool /* flush_data */)
    throw (IOException,
	   InternalException)
{
    /* No-op. */
}

DirectArrayFile::DirectArrayFile (MemoryDesc const &mem)
{
    array_mem = mem;

    pos = 0;
}

}

