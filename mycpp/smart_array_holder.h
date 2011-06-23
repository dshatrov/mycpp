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

#ifndef __MYCPP__SMART_ARRAY_HOLDER_H__
#define __MYCPP__SMART_ARRAY_HOLDER_H__

#include <cstddef> // for size_t

#include <mycpp/base_util.h>
#include <mycpp/mem_util.h>

namespace MyCpp {

template <class T>
class SmartArrayHolder
{
protected:
    T *array;
    unsigned long size;

private:
    SmartArrayHolder& operator = (const SmartArrayHolder &);
    SmartArrayHolder (const SmartArrayHolder &);

public:
    T& operator [] (const size_t index) const
    {
	abortIf (index >= size);

	return array [index];
    }

    unsigned long getSize ()
    {
	return size;
    }

    void copyToOffset (unsigned long  offset,
		       void          *from_buf,
		       unsigned long  len)
    {
	abortIf (offset + len < offset);
	abortIf (offset + len > size);

	copyMemory (array + offset, from_buf, len);
    }

    ComparisonResult compareTo (SmartArrayHolder<T> const &arr)
    {
	unsigned long pos = 0;
	for (;;) {
	    if (pos >= size) {
		if (pos >= arr.size)
		    return ComparisonEqual;

		return ComparisonLesser;
	    }

	    if (pos >= arr.size)
		return ComparisonGreater;

	    if (array [pos] > arr.array [pos])
		return ComparisonGreater;
	    else
	    if (array [pos] < arr.array [pos])
		return ComparisonLesser;

	    pos ++;
	}

	/* Unreachable */
	abortIfReached ();
	return ComparisonEqual;
    }

    void allocate (unsigned long size)
    {
	if (array != NULL)
	    delete[] array;

	if (size > 0)
	    array = new T [size];
	else
	    array = NULL;

	this->size = size;
    }

    void deallocate ()
    {
	if (array != NULL)
	    delete[] array;

	array = NULL;
	this->size = 0;
    }

    SmartArrayHolder ()
    {
	array = NULL;
	this->size = 0;
    }

    SmartArrayHolder (unsigned long size)
    {
	if (size > 0)
	    array = new T [size];
	else
	    array = NULL;

	this->size = size;
    }

    ~SmartArrayHolder ()
    {
	if (array != NULL)
	    delete[] array;
    }
};

}

#endif /* __MYCPP__SMART_ARRAY_HOLDER_H__ */

