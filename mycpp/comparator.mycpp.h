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

#ifndef __MYCPP__COMPARATOR_H__
#define __MYCPP__COMPARATOR_H__

#include <mycpp/extractor.h>
#include <mycpp/memory_desc.h>

namespace MyCpp {

/* A comparator should provide the following static methods:
 *
       static bool greater (L const &left, R const &right);
       static bool equals  (L const &left, R const &right);
   // TODO ComparisonResult compare (...);
 */

template < class T,
	   class LeftExtractor = DirectExtractor<T const &>,
	   class RightExtractor = LeftExtractor >
class DirectComparator
{
public:
    static bool greater (T const &left, T const &right)
    {
	return LeftExtractor::getValue (left) > RightExtractor::getValue (right);
    }

    static bool equals (T const &left, T const &right)
    {
	return LeftExtractor::getValue (left) == RightExtractor::getValue (right);
    }
};

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

#endif /* __MYCPP__COMPARATOR_H__ */

