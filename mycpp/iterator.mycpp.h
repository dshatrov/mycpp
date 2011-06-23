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

#ifndef __MYCPP__ITERATOR_H__
#define __MYCPP__ITERATOR_H__

#include <mycpp/types.h>
#include <mycpp/pointer.h>
#include <mycpp/extractor.h>
#include <mycpp/base_util.h>

namespace MyCpp {

// Stateful iterators must support direct copying for saving
// iterator's state.
template <class T>
class StatefulIterator_anybase
{
public:
    virtual T next () = 0;
    virtual bool done () = 0;

    virtual ~StatefulIterator_anybase () {}
};

template < class T,
	   class Base = EmptyBase >
class StatefulIterator : public StatefulIterator_anybase<T>,
			 public Base
{
};

// Resettable iterator
template <class T>
class IteratorBase
{
private:
    IteratorBase& operator = (IteratorBase &);
    IteratorBase (IteratorBase &);

public:
    virtual T next () = 0;
    virtual bool done () = 0;
    virtual void reset () = 0;

    IteratorBase () {}

    virtual ~IteratorBase () {}
};

// Generic iterators shouldn't support direct copying.
template < class T,
	   class Base = EmptyBase >
class Iterator : public IteratorBase<T>,
		 public Base
{
};

// This wrapper allows for representing stateful iterators
// as resettable iterators.
template < class T,
	   class StatefulIterator,
	   class Base = EmptyBase >
class IteratorWrapper : public Iterator<T, Base>
{
protected:
    StatefulIterator initial_state;
    StatefulIterator current_state;

public:
    T next ()
    {
	return current_state.next();
    }

    bool done ()
    {
	return current_state.done ();
    }

    void reset ()
    {
	current_state = initial_state;
    }

    IteratorWrapper (StatefulIterator const &iter)
	: initial_state (iter),
	  current_state (iter)
    {
    }
};

template < class T,                 // Iterator's return type
	   class Extractor,         // The extractor used for values returned by StatefulIterator
	   class StatefulIterator,  // The underlying iterator
	   class Base = EmptyBase >
class StatefulExtractorIterator : public MyCpp::StatefulIterator<T, Base>
{
protected:
    StatefulIterator iter;

public:
    T next ()
    {
	return Extractor::getValue (iter.next ());
    }

    bool done ()
    {
	return iter.done ();
    }

    StatefulExtractorIterator (StatefulIterator const &iter)
	: iter (iter)
    {
    }
};

template < class T,
	   class Extractor,
	   class Iterator,
	   class Base = EmptyBase >
class ExtractorIterator : public MyCpp::Iterator<T, Base>
{
protected:
    Iterator &iter;

public:
    T next ()
    {
	return Extractor::getValue (iter.next ());
    }

    bool done ()
    {
	return iter.done ();
    }

    void reset ()
    {
	iter.reset ();
    }

    ExtractorIterator (Iterator &iter)
	: iter (iter)
    {
    }
};

template < class T,
	   class Extractor,
	   class Iterator,
	   class Base = EmptyBase >
class RefExtractorIterator : public MyCpp::Iterator<T, Base>
{
protected:
    Ref<Iterator> iter;

public:
    T next ()
    {
	return Extractor::getValue (iter.next ());
    }

    bool done ()
    {
	return iter.done ();
    }

    void reset ()
    {
	iter.reset ();
    }

    RefExtractorIterator (Iterator *iter)
	: iter (iter)
    {
	abortIf (iter == NULL);
    }
};

template < class T,
	   class Base = EmptyBase >
class StatefulSingleItemIterator : public StatefulIterator<T&, Base>
{
protected:
    Pointer<T> item;
    bool iterated;

public:
    T& next ()
    {
	if (iterated)
	    abortIfReached ();

	iterated = true;
	return *item;
    }

    bool done ()
    {
	return iterated;
    }

    StatefulSingleItemIterator (Pointer<T> const &item)
	: item (item)
    {
	iterated = false;
    }
};

template < class T,
	   class Base = EmptyBase >
class SingleItemIterator : public IteratorWrapper< T&,
						   StatefulSingleItemIterator<T>,
						   Base >
{
public:
    SingleItemIterator (Pointer<T> const &item)
	: IteratorWrapper< T&,
			   StatefulSingleItemIterator<T>,
			   Base > (
		  StatefulSingleItemIterator<T> (item))
    {
    }
};

template < class T,
	   class Base = EmptyBase >
class IterArrayIterator : public Iterator<T, Base>
{
protected:
    IteratorBase<T> **iter_arr;
    unsigned long num_iters;

    unsigned long cur_iter;

public:
    T next ()
    {
	if (cur_iter >= num_iters)
	    abortIfReached ();

	while (iter_arr [cur_iter] == NULL ||
	       iter_arr [cur_iter]->done ())
	{
	    cur_iter ++;
	    if (cur_iter >= num_iters)
		abortIfReached ();
	}

	return iter_arr [cur_iter]->next ();
    }

    bool done ()
    {
	if (cur_iter >= num_iters)
	    return true;

	while (iter_arr [cur_iter] == NULL ||
	       iter_arr [cur_iter]->done ())
	{
	    cur_iter ++;
	    if (cur_iter >= num_iters)
		return true;
	}

	return false;
    }

    void reset ()
    {
	for (unsigned long i = 0; i < num_iters; i++) {
	    if (iter_arr [i] != NULL)
		iter_arr [i]->reset ();
	}

	cur_iter = 0;
    }

    IterArrayIterator (IteratorBase<T> **iter_arr,
		       unsigned long num_iters)
	: iter_arr (iter_arr),
	  num_iters (num_iters)
    {
	cur_iter = 0;
    }
};

template < class T,
	   class M = T&,
	   class Extractor = DirectExtractor<T&>,
	   class Base = EmptyBase >
class ArrayIterator : public Iterator<M, Base>
{
protected:
    T * const array;
    const unsigned long num_elements;

    unsigned long cur_element;

public:
    M next ()
    {
	if (cur_element >= num_elements)
	    abortIfReached ();

	T& ret = array [cur_element];
	cur_element ++;

	return Extractor::getValue (ret);
    }

    bool done ()
    {
	if (cur_element >= num_elements)
	    return true;

	return false;
    }

    void reset ()
    {
	cur_element = 0;
    }

    ArrayIterator (T *array,
		   unsigned long num_elements)
	: array (array),
	  num_elements (num_elements)
    {
	if (array == NULL && num_elements > 0)
	    abortIfReached ();

	cur_element = 0;
    }
};

}

#endif /* __MYCPP__ITERATOR_H__ */

