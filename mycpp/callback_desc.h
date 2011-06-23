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

#ifndef __MYCPP__CALLBACK_DESC_H__
#define __MYCPP__CALLBACK_DESC_H__

#include <mycpp/types.h>
#include <mycpp/referenced.h>
#include <mycpp/list.h>

namespace MyCpp {

/* NOTE: It _is_ assumed that 'operator =' works for CallbackDesc objects.
 *
 * NOTE: Calling setDesc() while holding a mutex for CallbackDesc initialized
 * with some external callback is generally unsafe, because this will trigger
 * the destructor for an object a reference to which is being held in 'cb_rdata' field.
 * The following pattern is suggested for re-initializing a CallbackDesc:
 *
 * void setCallback (const CallbackDesc<SomeCallback> *new_cb)
 * {
 *     Ref<SimplyReferenced> tmp_rdata;
 *     {
 *       MutexLock state_lock (state_mutex);
 *         tmp_rdata = cb.cb_rdata;
 *         cb.setDesc (new_cb);
 *     }
 *     tmp_rdata = NULL;
 * }
 *
 * NOTE: It is NOT guaranteed that 'refCallback' and 'unrefCallback' will be called
 * prior and after invocation of 'callback'. In certain circumstances calling 'refCallback'
 * makes no sense, mostly when the callback is set in the constructor and cannot
 * be changed after that.
 */
template <class T>
class CallbackDesc
{
public:
    // Avoid manipulating this list manually.
    // Use addRefData() method instead.
    List< Ref<M::Referenced> > rdata_list;

    WeakRef<Referenced> weak_obj;

    T            callback;
    void        *callbackData;

    RefCallback  refCallback;
    RefCallback  unrefCallback;
    void        *refData;

    void addRefData (M::Referenced *rdata)
    {
	rdata_list.append (rdata);
    }

    void setDesc (CallbackDesc<T> const *cd)
    {
	if (cd == NULL) {
	    reset ();
	} else {
	    weak_obj      = cd->weak_obj;
	    callback      = cd->callback;
	    callbackData  = cd->callbackData;
	    refCallback   = cd->refCallback;
	    unrefCallback = cd->unrefCallback;
	    refData       = cd->refData;

	    rdata_list.clear ();
	    List< Ref<M::Referenced> >::DataIterator iter (cd->rdata_list);
	    while (!iter.done ())
		rdata_list.append (iter.next ());
	}
    }

    void reset ()
    {
	weak_obj      = NULL;
	callback      = NULL;
	callbackData  = NULL;
	refCallback   = NULL;
	unrefCallback = NULL;
	refData       = NULL;

	rdata_list.clear ();
    }

    CallbackDesc& operator = (const CallbackDesc &cb)
    {
	setDesc (&cb);
	return *this;
    }

    CallbackDesc (const CallbackDesc &cb)
    {
	setDesc (&cb);
    }

    CallbackDesc ()
    {
	reset ();
    }
};

}

#endif /* __MYCPP__CALLBACK_DESC_H__ */

