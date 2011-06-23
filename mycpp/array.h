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

#ifndef __MYCPP__ARRAY_H__
#define __MYCPP__ARRAY_H__

#include <mycpp/simply_referenced.h>

namespace MyCpp {

/*c An interface for indexable groups of entities. */
template <class T>
class Array : public virtual SimplyReferenced
{
public:
    /* TODO Add a memcpy-like method to optimize
     * Array<->ArrayFile data transfers. */

    /*m Get the current size of the array.
     *
     * Returns the number of elements which are actually allocated
     * at the moment of the call. */
    virtual unsigned long getCurrentSize () = 0;

    /*m Get the maximum possible size for this array.
     *
     * In case of dynamic arrays, the size limit is usually
     * greater than the current size of the array. */
    virtual unsigned long getSizeLimit () = 0;

    /*m Get an element with a given index.
     *
     * If the index is greater or equal to the size limit of the array,
     * then the program is aborted.
     *
     * If the index is greater or equal to the current size of the array,
     * but less then array's size limit (dynamic array case)
     * then the array should be expanded, so that the index becomes
     * less than the array's current size.
     *
     * @index The index of the requested element. */
    virtual T& element (unsigned long index) = 0;
};

}

#endif /* __MYCPP__ARRAY_H__ */

