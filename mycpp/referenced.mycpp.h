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

#ifndef __MYCPP__REFERENCED_H__
#define __MYCPP__REFERENCED_H__

#include <mycpp/weak_ref.h>
#include <mycpp/mutex.h>
#include <mycpp/list.h>
#include <mycpp/callback_desc.h>

namespace MyCpp {

/*c <t>Referenced</t> objects' lifetime is controlled
 * by the mean of reference counting.
 *
 * A <t>Referenced</t> object destroys itself once its reference count
 * becomes zero.

 * After construction, the reference count of an object
 * is set to zero.
 *
 * In general, Referenced objects should not be allocated on the stack.
 *
 * See also documentation for <t>MyCpp::Ref</t> class,
 * which provides a convinient way of tracking references to
 * <t>Referenced</t> objects.
 *
 * All operations with <t>Referenced</t> objects (i.e. all the methods)
 * are atomic.
 */
class Referenced : public virtual SimplyReferenced
{
public:
    typedef void (*DeletionCallback) (void *data);

    class DeletionSubscription : public SimplyReferenced
    {
	friend class Referenced;

    protected:
	AtomicInt invalid;

	CallbackDesc<DeletionCallback> cb;

	List< Ref<DeletionSubscription> >::Element *obj_link;

	Ref<DeletionSubscription> mutual_sbn;

	Referenced *this_obj;
    };

private:
    Ref<Referenced_Shadow> shadow;

    Mutex stateMutex;

    bool sub_last_unref ();

    Mutex deletionMutex;
    List< Ref<DeletionSubscription> > deletion_subscriptions;

    static void mutualDeletionCallback (void *_sbn);

protected:
#if 0
NO DISPOSE
    /*m This is an important <b>protected</b> method,
     * which is called for an object just before it is
     * about to be destroyed (i.e. when its reference count
     * becomes zero, and the object is not floating).
     *
     * <i>Each class that overloads this method <b>must</b>
     * call the parent's <c>dispose</c> method before
     * returning.</i>
     *
     * During processing of the
     * <c>dispose</c> method, object's reference count
     * can be incremented, so that the destruction will
     * be avoided. In this case, the <c>dispose</c> method
     * will be called again, once the reference count reaches
     * zero again. This will be repeated until the object
     * gets actually destroyed.
     *
     * This method can be called multiple times,
     * but is guaranteed to be called only once at a time
     * (i.e. all calls are synchronous). */
    /* NOTE I'm deliberately making this class abstract.
     * In general, one should inherit either from class Object,
     * or from class SimplyReferenced. */
    virtual void _dispose () = 0;
#endif

public:
    Ref<Referenced_Shadow> getShadow ();

    Ref<DeletionSubscription> addDeletionCallback (const CallbackDesc<DeletionCallback> &cb_desc);

    Ref<DeletionSubscription> addDeletionCallbackNonmutual (const CallbackDesc<DeletionCallback> &cb_desc);

    void removeDeletionCallback (DeletionSubscription *sbn);

private:
    Referenced& operator = (Referenced const &);
    Referenced (Referenced const &obj);

public:
    Referenced ()
    {
    }

    ~Referenced ();
};

}

/* Includes for API */
#include <mycpp/ref.h>
#include <mycpp/weak_ref.h>
/* (End of includes for API) */

#endif /* __MYCPP__REFERENCED_H__ */

