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

#ifndef __MYCPP__COMPARATOR__H__
#define __MYCPP__COMPARATOR__H__


#include <libmary/comparator.h>

#include <mycpp/extractor.h>
#include <mycpp/memory_desc.h>
#include <mycpp/strutil.h>


namespace MyCpp {

using M::DirectComparator;

// TODO Make using M::MemoryComparator possible.

template < class T = ConstMemoryDesc const &,
	   class LeftExtractor = DirectExtractor<T>,
	   class RightExtractor = LeftExtractor >
class MemoryComparator
{
public:
    static bool greater (T left, T right)
    {
	return compareByteArrays (LeftExtractor::getValue (left), RightExtractor::getValue (right)) == ComparisonGreater;
    }

    static bool equals (T left, T right)
    {
	return compareByteArrays (LeftExtractor::getValue (left), RightExtractor::getValue (right)) == ComparisonEqual;
    }
};

}


#endif /* __MYCPP__COMPARATOR__H__ */

