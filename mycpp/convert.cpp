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

#include <stdlib.h>

#include <mycpp/convert.h>
#include <mycpp/util.h>

#include <glib.h>

namespace MyCpp {

bool string_to_double (const char *str, double *val)
{
    gchar *endptr;

    if (str [0] == 0)
	return false;

    *val = g_ascii_strtod (str, &endptr);

    if (*endptr != 0)
	return false;

    return true;
}

bool string_to_long (const char *str, long *val)
{
    size_t len;
    char *endptr;

    len = countStrLength (str);
    if (len <= 0)
	return false;

    *val = strtol (str, &endptr, 10);
    if (endptr != str + len)
	return false;

    return true;
}

}

