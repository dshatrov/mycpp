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

#ifndef __MYCPP__ATOMIC_POINTER_H__
#define __MYCPP__ATOMIC_POINTER_H__

namespace MyCpp {

class AtomicPointer
{
protected:
    void * /*gpointer*/ volatile ptr;

public:
    void set (void *ptr);

    void* get () const;

    bool compareAndExchange (void *oldPtr,
			     void *newPtr);

    AtomicPointer ();
};

}

#endif /* __MYCPP__ATOMIC_POINTER_H__ */

