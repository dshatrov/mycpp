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

/* Banning printf* functions
#include <stdarg.h>
*/

#include <mycpp/string.h>
#include <mycpp/print_task.h>
#include <mycpp/direct_array_file.h>
#include <mycpp/util.h>

namespace MyCpp {

static char _noData = 0;
char * const String::noData = &_noData;

#if 0
Banning printf* functions

Ref<String>
String::forFormat (const char *fmt, ...)
{
    String *string;
    char *str;
    va_list ap;

    va_start (ap, fmt);
    str = vmessage (fmt, ap);
    va_end (ap);

    string = grab (new String);
    string->setOwn (str);

    return string;
}
#endif

Ref<String>
String::catenate (Iterator<String&> &str_iter)
{
    unsigned long total_len = 0;

    while (!str_iter.done ()) {
	String &str = str_iter.next ();

	if (total_len + str.getLength () < total_len) {
	    /* TODO This is questionable. I don't want to force
	     * checking the return value, but abortIfReached()
	     * is no good either. */
	    abortIfReached ();
/*
	    oopsMessage ();
	    return NULL;
*/
	}
	total_len += str.getLength ();
    }

    if (total_len == 0)
	return String::nullString ();

    Ref<String> ret_str = grab (new String ());
    ret_str->allocate (total_len);

    str_iter.reset ();
    unsigned long pos = 0;
    while (!str_iter.done ()) {
	String &str = str_iter.next ();

	if (pos + str.getLength () < pos) {
	    abortIfReached ();
/*
	    oopsMessage ();
	    return NULL;
*/
	}

	if (pos + str.getLength () > total_len) {
	    abortIfReached ();
/*
	    oopsMessage ();
	    return NULL;
*/
	}

	copyMemory (ret_str->getMemoryDesc ().getRegionOffset (pos),
		    str.getMemoryDesc ());
	pos += str.getLength ();
    }

    ret_str->getData () [pos] = 0;

    return ret_str;
}

Ref<String>
String::forPrintTask (const PrintTask &pt)
{
    Ref<String> string = grab (new String ());
    unsigned long len = pt.chainLength ();
    /* len + 1 bytes will be allocated */
    string->allocate (len);

    DirectArrayFile daf (string->getMemoryDesc ());
    pt.chainPrint (&daf);
    daf.pflush ();
    string->getData () [len] = 0;

    return string;
}

}

