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

#ifndef __MYCPP__BUFFER_H__
#define __MYCPP__BUFFER_H__

#include <climits>

#include <mycpp/simply_referenced.h>
#include <mycpp/memory_desc.h>
#include <mycpp/print_task.h>

namespace MyCpp {

class LinearBufferInfo;

class Buffer : public virtual SimplyReferenced
{
public:
    // 08.10.11 NOTE: 'class Buffer' uses ad-hoc Vtable implementation for
    // virtual allocate/free methods. Usual virtual functions would do
    // as well, although there is a thin line with careful use of
    // virtuals in constructors and destructors.
    //
    // 'class String' (mycpp/string.h> is an example of implementation
    // which uses virtual functions.
    //
    // TODO: I think that using virtuals is a (much) better aproach.
    //
    class Vtable
    {
    public:
	void (*allocateMemory) (Buffer *self,
				Size    len);

	void (*freeMemory) (Buffer *self);
    };

// protected:
public:
    Byte *data;
    Size  size;

    Vtable *vtable;

public:
    Byte& operator [] (Size index) const
    {
	abortIf (index >= size);
	return data [index];
    }

    // Usage of getData() is discouraged, hence the _unsafe suffix.
    // Use getMemoryDesc() instead.
    Byte* getData_unsafe () const
    {
	return data;
    }

    Size getSize () const
    {
	return size;
    }

// getMemoryDesc() should not be overloaded with empty parameter list because
// this way it can be easily confused with LinearBufferInfo::getMemoryDesc().
//    MemoryDesc getMemoryDesc () const;

    MemoryDesc getMemoryDesc (Size start_offset,
			      Size len) const;

    void allocate (Size len);

    void writeMemory (ConstMemoryDesc const &mdesc,
		      Size start_offset,
		      Size len);

    static LinearBufferInfo createNew (Size len);

    // Returns a null reference if the print task does not fit
    // into maxSize bytes.
    static LinearBufferInfo forPrintTask (const PrintTask &pt,
					  Size maxSize = ULONG_MAX);

    static LinearBufferInfo forData (ConstMemoryDesc const &mdesc);

    Buffer (Size len);

    Buffer ();

    virtual ~Buffer ();
};

class Buffer_ExtAlloc : public Buffer
{
public:
    typedef void (*FreeCallback) (void *mem);

// protected:
public:
    FreeCallback free_callback;

public:
    Buffer_ExtAlloc (Byte         *ext_data,
		     Size          ext_size,
		     FreeCallback  ext_free_callback);

    ~Buffer_ExtAlloc ();
};

}

#include <mycpp/linear_buffer_info.h>

#endif /* __MYCPP__BUFFER_H__ */

