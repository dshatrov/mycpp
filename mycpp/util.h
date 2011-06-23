/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2010 Dmitry Shatrov
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

#ifndef __MYCPP__UTIL_H__
#define __MYCPP__UTIL_H__

#include <mycpp/base_util.h>
#include <mycpp/mem_util.h>
#include <mycpp/strutil.h>

#include <mycpp/string.h>
#include <mycpp/file.h>

#ifdef PLATFORM_WIN32
#include <Windows.h>
#endif

namespace MyCpp {

#ifndef PLATFORM_WIN32
Ref<String> errnoToString (int code);
#else
Ref<String> win32ErrorToString (DWORD error);
#endif

void printError (const char *str);

Ref<String> collectBacktrace ();

void printBacktrace (File *out);

void printException (File *out,
		     Exception const &exc,
		     const char *msg = NULL);

void uSleep (unsigned long microseconds);

Uint32 randomUint32 ();

void hexDump (File *file,
	      ConstMemoryDesc const &mdesc);

void hexDump (File                *file,
	      const unsigned char *buf,
	      unsigned long        len);

}

#endif /*__MYCPP__UTIL_H__*/

