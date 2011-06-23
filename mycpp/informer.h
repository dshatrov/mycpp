/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2008  Dmitry M. Shatrov
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

#ifndef __MYCPP__INFORMER_H__
#define __MYCPP__INFORMER_H__

#include <mycpp/types.h>
#include <mycpp/callback_desc.h>
#include <mycpp/list.h>
#include <mycpp/object.h>
#include <mycpp/data_mutex.h>

namespace MyCpp {

/*c Informers provide a consistent way for registering callbacks
 * and issuing synchronous events.
 *
 * A single informer manages callbacks for a single event. */
class GenericInformer : public virtual Object
{
public:
    /*t*/
    typedef void (*InformCallback) (VoidFunction callback,
				    void *callbackData,
				    void *data);

    enum InformFlags {
	InformOneshot = 1,
	InformSuspended = 2
    };

    class Subscription : public virtual SimplyReferenced
    {
	friend class GenericInformer;

    protected:
      // mt_const

	GenericInformer *informer;

	bool oneshot;

	List< Ref<Subscription> >::Element *sbnsLink;

      // (end mt_const)

      // mt_mutex Informer::state_mutex

	bool  valid;

	bool suspended;

	bool  inSuspendedList;

	// NOTE: This is a dangerous hidden reference.
	// Special care must be taken to call 'cb.reset()' once
	// the Subscription object is detached from its parent
	// GenericInformer for any reason (client unsubscribes,
	// GenericInformer is being destroyed, etc.)
	CallbackDesc<VoidFunction> cb;
	Object::DeletionSubscriptionKey deletion_sbn;

      // (end mt_mutex Informer::state_mutex)
    };

#if 0
// There's no need in this currently.

    class SubscriptionHolder : public virtual SimplyReferenced
    {
    protected:
	Ref<GenericInformer> informer;
	Ref<Subscription> subscription;

    public:
	SubscriptionHolder (GenericInformer *informer,
			    Subscription    *subscription)
	{
	    this->informer = informer;
	    this->subscription = subscription;
	}

	~SubscriptionHolder ()
	{
	    if (!informer.isNull () &&
		!subscription.isNull ())
	    {
		informer->unsubscribe (subscription);
	    }
	}
    };
#endif

protected:
  // mt_const

    InformCallback informCallback;

  // (end mt_const)

  // mt_mutex state_mutex

    List< Ref<Subscription> > subscriptions;
    List< Ref<Subscription> > suspendedSbns;

    Size traversing;

  // end mt_mutex state_mutex

    DataMutex state_mutex;

    void cleanupSubscription (Subscription *sbn);

    static void subscriberDeletionCallback (void *_sbn);

    // This method allows one to subscribe and forget about it.
    Ref<Subscription> subscribeVoid (const CallbackDesc<VoidFunction> &cb,
				     unsigned long flags = 0);

    /*m Triggers the event.
     *
     * When this method gets called all subscriptions'
     * callbacks get called.
     *
     * @data Data to be passed to <c>informCallback</c>. */
    void informAll (void *data);

public:
    /*m*/
    void unsubscribe (Subscription *sbn);

    /*m
     *
     * suspendSubscription() must not trigger invocation of event callbacks. */
    void suspendSubscription (Subscription *sbn);

    /*m
     *
     * resumeSubscription() must not trigger invocation of event callbacks. */
    void resumeSubscription (Subscription *sbn);

    /*m The constructor.
     *
     * @informCallback A callback to be called to inform
     * a subscriber about the event. */
    GenericInformer (InformCallback informCallback);

    ~GenericInformer ();
};

template <class T>
class Informer : public GenericInformer
{
public:
    /*t*/
    typedef void (*InformCallback) (T callback,
				    void *callbackData,
				    void *data);

protected:
    struct InformData
    {
	Informer *informer;
	void     *data;
    };

    InformCallback inform_callback;

    static void proxyInformCallback (VoidFunction callback,
				     void *callback_data,
				     void *_idata)
    {
	InformData *idata = static_cast <InformData*> (_idata);

	idata->informer->inform_callback (reinterpret_cast <T> (callback),
					  callback_data,
					  idata->data);
    }

public:
    void informAll (void *data)
    {
	InformData idata;
	idata.informer = this;
	idata.data = data;

	GenericInformer::informAll (static_cast <void*> (&idata));
    }

    Ref<Subscription> subscribe (const CallbackDesc<T> &cb,
				 unsigned long flags = 0)
    {
	// NOTE: This code (almost) duplicates CallbackDesc.setDesc(), which is no good.

	CallbackDesc<VoidFunction> vcb;
	vcb.weak_obj      = cb.weak_obj;
	vcb.callback      = reinterpret_cast <VoidFunction> (cb.callback);
	vcb.callbackData  = cb.callbackData;
	vcb.refCallback   = cb.refCallback;
	vcb.unrefCallback = cb.unrefCallback;
	vcb.refData       = cb.refData;

	vcb.rdata_list.clear ();
	List< Ref<M::Referenced> >::DataIterator iter (cb.rdata_list);
	while (!iter.done ())
	    vcb.addRefData (iter.next ());

	return subscribeVoid (vcb, flags);
    }

    Informer (InformCallback inform_callback)
	: GenericInformer (proxyInformCallback)
    {
	this->inform_callback = inform_callback;
    }
};

}

#endif /* __MYCPP__INFORMER_H__ */

