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

#ifndef __MYCPP__EMBEDDED_LIST_H__
#define __MYCPP__EMBEDDED_LIST_H__

#include <mycpp/types.h>

namespace MyCpp {

/* Note: Embedded lists are usable for POD types only. */

/* NOTE Conversion of a pointer to a POD type to 'unsigned char*' is
 * an implementation-defined reinterpret_cast, but such conversions
 * are reasonable thanks to the wording of paragraphs about representation
 * of types (C++98, 3.9, see also 9.2#17). */
#define MYCPP_CONTAINER_OF(ptr, type, member)		\
    (reinterpret_cast <type*> (				\
	    reinterpret_cast <unsigned char*> (ptr) -	\
		offsetof (type, member)))

/* Embed only into POD aggregate types (C++98, 9#4).
 * This must be a POD-struct itself. */
struct EmbeddedListElement
{
    EmbeddedListElement *next,
			*previous;
};

/* This is a circular list. */
struct EmbeddedListHead
{
    EmbeddedListElement *first;

    EmbeddedListHead () {
	first = NULL;
    }
};

void embeddedListAppend (EmbeddedListHead    *head,
			 EmbeddedListElement *new_el,
			 EmbeddedListElement *old_el);

void embeddedListPrepend (EmbeddedListHead    *head,
			  EmbeddedListElement *new_el,
			  EmbeddedListElement *old_el);

void embeddedListRemove (EmbeddedListHead    *head,
			 EmbeddedListElement *el);

}

#endif /* __MYCPP__EMBEDDED_LIST_H__ */

