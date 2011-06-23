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

#ifndef __MYCPP__WEAK_REF_H__
#define __MYCPP__WEAK_REF_H__

#include <mycpp/simply_referenced.h>
#include <mycpp/rw_lock.h>
#include <mycpp/ref.h>

namespace MyCpp {

class Referenced

class Referenced_Shadow : public SimplyReferenced
{
    friend class Referenced;

public:
    RwLock rw_lock;
    Referenced *weak_ptr;

    Referenced_Shadow ()
	: weak_ptr (NULL)
    {
    }
};

// Note that WeakRef objects are not thread-safe. Accesses to them should be
// synchronized externally.
template <class T>
class WeakRef
{
protected:
    Ref<Referenced_Shadow> shadow;

    /* This pointer is valid only when shadow->weak_ptr is non-NULL.
     * It is used to avoid dynamic_cast'ing shadow->weak_ptr, which is
     * of type 'Referenced*' in order to serve WeakRef<T> with any T. */
    T *typed_weak_ptr;

    void _subscribe (T *obj)
    {
	if (obj != NULL) {
	    typed_weak_ptr = obj;
	    shadow = obj->getShadow ();
	}
    }

    void _unsubscribe ()
    {
	shadow = NULL;
	typed_weak_ptr = NULL; // not strictly necessary
    }

public:
    // For debugging purposes only.
    Ref<Referenced_Shadow> getShadow ()
    {
	return shadow;
    }

    Ref<T> getRef () const
    {
	if (shadow.isNull ())
	    return NULL;

	shadow->rw_lock.readerLock ();

	Ref<T> ref;
	if (shadow->weak_ptr != NULL)
	    ref = typed_weak_ptr;

      /* DEBUG */
	if (shadow->weak_ptr != NULL &&
	    ref.isNull ())
	{
	    abortIfReached ();
	}
      /* (DEBUG) */

	shadow->rw_lock.readerUnlock ();

	return ref;
    }

    bool isValid () const
    {
	return !shadow.isNull ();
    }

    bool isNull () const
    {
	if (!shadow.isNull ()) {
	    shadow->rw_lock.readerLock ();
	    bool is_null = (shadow->weak_ptr == NULL);
	    shadow->rw_lock.readerUnlock ();

	    return is_null;
	}

	return true;
    }

    WeakRef& operator = (T *obj)
    {
	_unsubscribe ();
	_subscribe (obj);
	return *this;
    }

#if 0
This is equivalent to automatically-generated methods.
    WeakRef& operator = (WeakRef const &wr)
    {
	shadow = wr.shadow;
	typed_weak_ptr = wr.typed_weak_ptr;
    }

    WeakRef (WeakRef const &wr)
    {
	shadow = wr.shadow;
	typed_weak_ptr = wr.typed_weak_ptr;
    }
#endif

    WeakRef (T *obj)
	: typed_weak_ptr (NULL) // not strictly necessary
    {
	_subscribe (obj);
    }

    WeakRef ()
	: typed_weak_ptr (NULL) // not strictly necessary
    {
    }

    ~WeakRef ()
    {
	_unsubscribe ();
    }
};

}

#endif /* __MYCPP__WEAK_REF_H__ */

