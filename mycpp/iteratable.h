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

#ifndef __MYCPP__ITERATABLE_H__
#define __MYCPP__ITERATABLE_H__

#include <mycpp/extractor.h>
#include <mycpp/iterator.h>
#include <mycpp/list.h>
#include <mycpp/simply_referenced.h>
#include <mycpp/base_util.h>

namespace MyCpp {

template <class T>
class Iteratable : public virtual SimplyReferenced
{
public:
    virtual Ref< Iterator<T, SimplyReferenced> > createIterator () const = 0;
};

template < class T,
	   class M = T&,
	   class Extractor = DirectExtractor<T&> >
class Iteratable_SingleRef : public Iteratable<M>,
			     public virtual SimplyReferenced
{
protected:
    Ref<T> item;

public:
    Ref< Iterator<M, SimplyReferenced> > createIterator () const
    {
	return grab (static_cast < Iterator<M, SimplyReferenced> * > (
		new IteratorWrapper< M,
				     StatefulExtractorIterator< M,
								Extractor,
								StatefulSingleItemIterator<T> >,
				     SimplyReferenced > (
			StatefulExtractorIterator< M,
						   Extractor,
						   StatefulSingleItemIterator<T> > (
				StatefulSingleItemIterator<T> (item)))));
    }

    Iteratable_SingleRef (T *item)
    {
	abortIf (item == NULL);
	this->item = item;
    }
};

template < class T,
	   class M = T,
	   class Extractor = DirectExtractor<T> >
class Iteratable_List : public Iteratable<M>,
			public virtual SimplyReferenced
{
protected:
    Ref< List_<T, SimplyReferenced> > list;

public:
    Ref< Iterator<M, SimplyReferenced> > createIterator () const
    {
	return grab (static_cast < Iterator<M, SimplyReferenced> * > (
		new IteratorWrapper < M,
				      StatefulExtractorIterator < M,
								  Extractor,
								  typename List<T>::DataIterator >,
				      SimplyReferenced > (
			StatefulExtractorIterator < M,
						    Extractor,
						    typename List<T>::DataIterator > (
				list->template createDataIterator<EmptyBase> ()))));
    }

    Iteratable_List (List_<T, SimplyReferenced> *list)
    {
	abortIf (list == NULL);
	this->list = list;
    }
};

}

#endif /* __MYCPP__ITERATABLE_H__ */

