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

#ifndef __MYCPP_EXTERNAL_ARRAY_H__
#define __MYCPP_EXTERNAL_ARRAY_H__

#include <mycpp/array.h>
#include <mycpp/iterator.h>
#include <mycpp/util.h>

namespace MyCpp {

/*c A wrapper for conventional C arrays.
 *
 * A static (i.e., non-resizable) <t>Array</t>
 * relying on a conventional C array.
 *
 * The array is not copied, thus the original C array
 * should persist during lifetime of the corresponding
 * <t>ExternalArray</t> object.
 */
template <class T>
class ExternalArray : public Array <T>
{
public:
    class Iterator : public MyCpp::Iterator <T&>
    {
    protected:
	ExternalArray &array;
	unsigned long cur_index;

    public:
	T& next ()
	{
	    if (cur_index >= array.getCurrentSize ())
		abortIfReached ();

	    T &ret = array.element (cur_index);

	    cur_index ++;

	    return ret;
	}

	bool done ()
	{
	    if (cur_index >= array.getCurrentSize ())
		return true;

	    return false;
	}

	void reset ()
	{
	    cur_index = 0;
	}

	Iterator (ExternalArray &_array)
	    : array (_array)
	{
	    cur_index = 0;
	}
    };

protected:
    T *array;
    unsigned long size;

public:
  /* Array interface */

    T& element (unsigned long num) {
	if (num >= size) {
	    abortIfReached_str ("Array index out of range");
	}

	return array [num];
    }

    unsigned long getCurrentSize () {
	return size;
    }

    unsigned long getSizeLimit () {
	return size;
    }

  /* (End of Array interface) */

    /*m The constructor.
     *
     * @array An array to wrap. May NOT be NULL unless <i>size</i> is 0.
     * @size  The number of elements in the wrapped array. */
    ExternalArray (T *array,
		   unsigned long size)
    {
	if (array == NULL &&
	    size > 0)
	{
	    abortIfReached_str ("NULL array pointer");
	}

	this->array = array;
	this->size = size;
    }
};

}

#endif /* __MYCPP_EXTERNAL_ARRAY_H__ */

