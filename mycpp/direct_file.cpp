/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006  Dmitry M. Shatrov
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

#include <errno.h>

#include <mycpp/util.h>
#include <mycpp/direct_file.h>

namespace MyCpp {

/* TODO Since current implementation uses fopen() and other f*()
 * functions to work with files, it's not actualy non-buffered
 * as stated in the description for DirectFile.
 *
 * This should be rewritten to use POSIX system calls. */

unsigned long
DirectFile::read (char          *buf,
		  unsigned long  len)
    throw (IOException)
{
    size_t res;

    res = fread (buf, 1, len, file);
    if (res == 0) {
	if (feof (file))
	    return 0;

	throw IOException (errnoToString (errno));
    }

    return res;
}

unsigned long
DirectFile::write (const char    *buf,
		   unsigned long  len)
    throw (IOException)
{
    size_t res;

    res = fwrite (buf, len, 1, file);
    if (res != 1)
	throw IOException (errnoToString (errno));

    return len;
}

void
DirectFile::seek (long long  offset,
		  SeekOrigin origin)
    throw (IOException)
{
    if (fseeko (file, offset,
		origin == SeekCur ? SEEK_CUR :
		origin == SeekBeg ? SEEK_SET :
				    SEEK_END))
    {
	throw IOException (errnoToString (errno));
    }
}

long long
DirectFile::tell ()
    throw (IOException)
{
    off_t off;
       
    off = ftello (file);
    if (off == -1)
	throw IOException (errnoToString (errno));

    return off;
}

void
DirectFile::flush ()
    throw (IOException)
{
    if (fflush (file) == EOF)
	throw IOException (errnoToString (errno));
}

const char*
DirectFile::modeToMstring (AccessMode mode)
{
    if (mode == ReadWrite)
	return "r+b";
    else
    if (mode == ReadOnly)
	return "rb";
    else
    if (mode == WriteOnly)
	/* Personally, I think that fopen() modes in ANSI C
	 * are brain-damaged. There is no way to open a file
	 * exclusively for writing without truncating it.
	 *     - DMS */
	return "r+b";
    else
	abortIfReached (grab (new String ("Invalid file access mode")));

    /* unreachable */
    return "rb";
}

DirectFile::DirectFile (const char *filename,
			AccessMode mode)
    throw (IOException)
{
    const char *mstring = modeToMstring (mode);

    file = fopen (filename, mstring);
    if (file == NULL)
	throw IOException (errnoToString (errno));
}

DirectFile::DirectFile (int fd,
			AccessMode mode)
    throw (IOException)
{
    const char *mstring = modeToMstring (mode);

    file = fdopen (fd, mstring);
    if (file == NULL)
	throw IOException (errnoToString (errno));
}

DirectFile::~DirectFile ()
{
    fclose (file);
}

};

