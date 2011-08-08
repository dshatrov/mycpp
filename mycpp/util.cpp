/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2008  Dmitry M. Shatrov
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

#include <stdlib.h>

#ifdef __linux__
#include <execinfo.h> /* for backtraces */
#endif

//#include <unistd.h> /* for write() */

#include <mycpp/util.h>
#include <mycpp/io.h>

#include <glib.h>

namespace MyCpp {

void
uSleep (unsigned long microseconds)
{
    g_usleep ((gulong) microseconds);
}

Uint32
randomUint32 ()
{
    /* From Glib docs on g_random_int():
     * "Return a random guint32 equally distributed over the range [0..2^32-1]." */
    return (Uint32) g_random_int ();
}

void
_abortIfReached (const char    *file,
		 unsigned long  line,
		 const char    *str)
{
    /* TODO Don't use 'errf' here, rely on pure write() calls. */

    failmodeErrf->
	  print ("\n**** MyCpp ****\n"
		 "** Unconditional abort point reached\n"
		 "** file ")
	 .print (file)
	 .print (", line ")
	 .print (line)
	 .print ("\n**\n");
   
    if (str != NULL)
	failmodeErrf->
	      print ("** ")
	     .print (str)
	     .print ("\n");
   
    failmodeErrf->pendl ();

#ifdef PLATFORM_WIN32
    *((char*) 0) = 0;
    abort ();
#else
    abort ();
#endif
}

void
_oopsMessage (const char    *file,
	      unsigned long  line,
	      const char    *str)
{
    errf->print ("\n**** MyCpp ****\n"
		 "** Oops (run-time anomaly, most likely a programming bug\n"
		 "** file ")
	 .print (file)
	 .print (", line ")
	 .print (line)
	 .print ("\n**\n");
   
    if (str != NULL)
	errf->print ("** ")
	     .print (str)
	     .print ("\n");
   
    errf->pendl ();
}

char*
rawCollectBacktrace ()
{
#ifdef __linux__
    const char *fail_str = "[Failed to get the backtrace]";

    void *addrs [128];
    int naddrs = backtrace (addrs, 128);
    if (naddrs < 0 ||
	naddrs > 128)
    {
	errf->print ("MyCpp.collectBacktrace: unexpected return value "
		     "from backtrace()")
	     .pendl ();

	Size fail_len = countStrLength (fail_str);
	char *bt = new char [fail_len + 1];
	copyMemory (MemoryDesc (bt, fail_len + 1), ConstMemoryDesc (fail_str, fail_len + 1));
	return bt;
    }

    if (naddrs == 128) {
	errf->print ("MyCpp.collectBacktrace: the backtrace is possibly "
		     "truncated, 128 entries max")
	     .pendl ();
    }

    char **bt_arr = backtrace_symbols (addrs, naddrs);
    if (bt_arr == NULL) {
	errf->print ("MyCpp.collectBacktrace: backtrace_symbols() failed")
	     .pendl ();

	Size fail_len = countStrLength (fail_str);
	char *bt = new char [fail_len + 1];
	copyMemory (MemoryDesc (bt, fail_len + 1), ConstMemoryDesc (fail_str, fail_len + 1));
	return bt;
    }

    unsigned long i;
    unsigned long total_len = 0;

    for (i = 0; i < (unsigned long) naddrs; i++) {
	if (bt_arr [i] == NULL)
	    continue;

	unsigned long str_len = countStrLength (bt_arr [i]);
	if (total_len + str_len <= total_len)
	    return NULL;
	total_len += str_len;
	/* Plus one byte for a newline. */
	if (total_len + 1 <= total_len)
	    return NULL;
	total_len += 1;
    }

    if (total_len > 0) {
	/* Last newline is unnecessary. */
	total_len --;
    }

    if (total_len == 0)
	return NULL;

    char *backtrace = new char [total_len + 1];

    unsigned long pos = 0;
    for (i = 0; i < (unsigned long) naddrs; i++) {
	if (bt_arr [i] == NULL)
	    continue;

	unsigned long str_len = countStrLength (bt_arr [i]);
	copyMemory (MemoryDesc (backtrace + pos, total_len - pos),
		    ConstMemoryDesc (bt_arr [i], str_len));
	pos += str_len;
	/* String.allocate() allocates one additional byte
	 * for the trailing zero. The last newline will
	 * go exactly to that byte. */
	backtrace [pos] = '\n';
	pos ++;
    }
    backtrace [total_len] = 0;

    free (bt_arr);

    return backtrace;
#else
    return NULL;
#endif
}

Ref<String>
collectBacktrace ()
{
    Ref<String> backtrace = grab (new String ());
    backtrace->setOwn (rawCollectBacktrace ());
    return backtrace;
}

void
printBacktrace (File *out)
{
    if (out == NULL)
	return;

    Ref<String> bt = collectBacktrace ();
    if (bt.isNull ()) {
	out->print ("[Failed to collect the backtrace]").pendl ();
	return;
    }

    out->print (bt->getData ()).pendl ();
}

void
printException (File *out,
		Exception const &exc,
		const char *msg)
{
    if (out == NULL)
	return;

/* Exception::getMessage() returns a string of concatenated
 * exception messages, so this code is not needed.
 *
    out->print (msg != NULL ? msg : "Exception: ");

    Exception const *cur_exc = &exc;
    while (cur_exc != NULL) {
	Ref<String> msg = exc.getMessage ();
	if (!msg.isNull () && !msg->isNullString ())
	    out->print (msg).print (": ");

	cur_exc = cur_exc->getCause ();
    }

    out->print ("\nBacktrace:\n")
	.print (exc.getBacktrace ())
	.pendl ();
*/

    out->print (msg != NULL ? msg : "Exception: ")
	.print (exc.getMessage ())
	.print ("\nBacktrace:\n")
	.print (exc.getBacktrace ())
	.pendl ();
}

static void
printAscii (File *file,
	    unsigned char c)
{
    static const char ascii_tab [256] = {
      //  0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', //  0
	'.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', //  1
	'.', '!', '"', '#', '$', '%', '&', '\'','(', ')', '*', '+', ',', '-', '.', '/', //  2
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', //  3
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', //  4
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_', //  5
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', //  6
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '.'  //  7
    };

    if (c > 127)
	file->print (".");
    else
	file->print (ConstMemoryDesc (&ascii_tab [(unsigned long) c], 1));
}

static void
printHexTab (File *file,
	     unsigned long val,
	     unsigned long tab)
{
    unsigned long len = file->noutHex (val);

    unsigned long offs = tab - len % tab;
    if (offs > 0) {
	for (unsigned long i = 0; i < offs; i++)
	    file->print (" ");
    }

    file->printHex (val);
}

void
hexDump (File *file,
	 ConstMemoryDesc const &mdesc)
{
    hexDump (file, mdesc.getMemory (), mdesc.getLength ());
}

void
hexDump (File                *file,
	 const unsigned char *buf,
	 unsigned long        len)
{
    if (file == NULL ||
	buf == NULL)
    {
	abortIfReached ();
    }

    const unsigned long row_width = 8;

    file->print ("       ");
    for (unsigned long i = 0; i < row_width; i++)
	printHexTab (file, i, 4);
    file->print ("\n");

    unsigned long cnt = 0;
    unsigned long pos;
    for (pos = 0; pos < len; pos++) {
	if (cnt % row_width == 0) {
	    printHexTab (file, (unsigned long) pos, 4);
	    file->print (" | ");
	    cnt = 0;
	}

	printHexTab (file, (unsigned long) buf [pos], 4);

	cnt++;
	if (cnt % row_width == 0) {
	    file->print ("   ");
	    for (unsigned long i = 0; i < row_width; i++)
		printAscii (file, buf [pos - (row_width - 1) + i]);

	    file->print ("\n");
	}
    }

    if (cnt % row_width != 0) {
	for (unsigned long i = 0; i < row_width - cnt; i++)
	    file->print ("    ");

	file->print ("   ");
	for (unsigned long i = 0; i < cnt; i++)
	    printAscii (file, buf [pos - cnt + i]);

	file->print ("\n");
    }

    if (cnt % row_width != 0)
//	file->pendl ();
	file->flush ();
    else
	file->flush ();
}

}

