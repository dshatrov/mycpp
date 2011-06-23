/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2009-2010 Dmitry Shatrov
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

#ifndef __MYCPP__IO_H__
#define __MYCPP__IO_H__

#include <mycpp/file.h>

/* print_task.h is included intentionally
 * to avoid the need to include both io.h and print_task.h */
#include <mycpp/print_task.h>

namespace MyCpp {

extern Ref<File> stdoutFile;
extern Ref<File> stderrFile;

/* Shorter synonims. */
extern File *outf;
extern File *errf;

extern File *failmodeOutf;
extern File *failmodeErrf;

}

#endif /* __MYCPP__IO_H__ */

