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

#include <mycpp/informer.h>
#include <mycpp/util.h>
#include <mycpp/io.h>

#define DEBUG(a) ;

namespace MyCpp {

void
GenericInformer::cleanupSubscription (Subscription * const sbn)
{
    state_mutex.assertLocked ();
    abortIf (sbn == NULL);

    // if (weak_obj.isValid()) would be more appropriate.
    if (sbn->deletion_sbn) {
	Ref<Referenced> obj;
	obj = sbn->cb.weak_obj.getRef ();
	if (obj)
	    obj->removeDeletionCallback (sbn->deletion_sbn);

	sbn->deletion_sbn = NULL;
    }

    sbn->cb.reset ();
}

void
GenericInformer::subscriberDeletionCallback (void *_sbn)
{
    Subscription * const &sbn = static_cast <Subscription*> (_sbn);
    GenericInformer *self = sbn->informer;

    self->unsubscribe (sbn);
}

Ref<GenericInformer::Subscription>
GenericInformer::subscribeVoid (const CallbackDesc<VoidFunction> &cb,
				unsigned long flags)
{
    Ref<Referenced> obj;
    if (cb.weak_obj.isValid ()) {
	obj = cb.weak_obj.getRef ();
	if (!obj)
	    return NULL;
    }

    Ref<Subscription> s = grab (new Subscription);
    s->valid = true;
    s->informer = this;
    s->oneshot = flags & InformOneshot;
    s->suspended = flags & InformSuspended;
    s->cb.setDesc (&cb);

    if (obj) {
	s->deletion_sbn = obj->addDeletionCallbackNonmutual (
                                  M::CbDesc<Object::DeletionCallback> (
                                          subscriberDeletionCallback,
                                          s,
                                          this,
                                          s));
    }

    state_mutex.lock ();

    if (s->suspended) {
	suspendedSbns.append (s, suspendedSbns.last);
	s->sbnsLink = suspendedSbns.last;
	DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.subscribeVoid: inSuspendedList = true").pendl (); )
	s->inSuspendedList = true;
    } else {
	subscriptions.append (s, subscriptions.last);
	s->sbnsLink = subscriptions.last;
	s->inSuspendedList = false;
    }

    state_mutex.unlock ();

    return s;
}

void
GenericInformer::unsubscribe (Subscription *sbn)
{
    DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.unsubscribe, sbn: ").print (sbn).pendl (); )

    if (sbn == NULL)
	return;

  DataMutexLock state_lock (state_mutex);

    if (!sbn->valid) {
	DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.unsubscribe: subscription is invalid, sbn: ").print (sbn).pendl (); )
	return;
    }
    sbn->valid = false;

    if (!sbn->inSuspendedList) {
	DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.unsubscribe: !inSuspendedList").pendl (); )
	if (traversing == 0) {
	    DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.unsubscribe: traversing == 0, removing").pendl (); )
	    subscriptions.remove (sbn->sbnsLink);
	    cleanupSubscription (sbn);
	} else {
	    DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.unsubscribe: traversing != 0").pendl (); )
	    // Shouldn't cleanup the subscription here, because
	    // objects in its 'rdata_list' should remain referenced
	    // while the callback is called.
	}
    } else {
	DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.unsubscribe: inSuspendedList").pendl (); )
	suspendedSbns.remove (sbn->sbnsLink);
	cleanupSubscription (sbn);
    }
}

void
GenericInformer::suspendSubscription (Subscription *sbn)
{
    DEBUG (errf->print (this).print (" MyCpp.GenericInformer.suspendSubscription").pendl (); )

    if (sbn == NULL)
	return;

  DataMutexLock state_lock (state_mutex);

    if (!sbn->valid)
	return;

    if (!sbn->suspended) {
	sbn->suspended = true;

	if (traversing == 0) {
	    subscriptions.remove (sbn->sbnsLink);
	    suspendedSbns.append (sbn, suspendedSbns.last);
	    sbn->sbnsLink = suspendedSbns.last;
	    DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.suspendSubscription: inSuspendedList = true").pendl (); )
	    sbn->inSuspendedList = true;
	}
    }
}

void
GenericInformer::resumeSubscription (Subscription *sbn)
{
    if (sbn == NULL)
	return;

  DataMutexLock state_lock (state_mutex);

    if (!sbn->valid)
	return;

    if (sbn->suspended) {
	sbn->suspended = false;

	if (sbn->inSuspendedList) {
	    suspendedSbns.remove (sbn->sbnsLink);
	    subscriptions.append (sbn, subscriptions.last);
	    sbn->sbnsLink = subscriptions.last;
	    sbn->inSuspendedList = false;
	}
    }
}

void
GenericInformer::informAll (void *data)
{
  // It is safe to append an element to the end of
  // subscription list even during traveresal of
  // subscriptions list in informAll method.

    // Note: It is vitally important to keep us from being destroyed as a result
    // of some callback invocation. This applies to all methods which invoke callbacks
    // and rely on persistence of some externally-held references.
    Ref<GenericInformer> self_ref (this);

    state_mutex.lock ();

    List< Ref<Subscription> >::Element *sbnEl, *nextSbnEl;
    Subscription *sbn;

    traversing ++;
    sbnEl = subscriptions.first;
    while (sbnEl != NULL) {
	sbn = sbnEl->data;
	if (sbn->valid &&
	    !sbn->suspended)
	{
	    if (sbn->oneshot) {
		// cleanupSubscriptions_unlocked() is called below
		DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.informAll: invalidating oneshot sbn: ").print (sbn).pendl (); )
		sbn->valid = false;
	    }

	    bool skip = false;
	    Ref<Referenced> obj;
	    // if (weak_obj.isValid()) would be more appropriate.
	    if (sbn->deletion_sbn) {
		obj = sbn->cb.weak_obj.getRef ();
		if (!obj)
		    skip = true;
	    }

	    if (!skip) {
		if (sbn->cb.refCallback != NULL)
		    sbn->cb.refCallback (sbn->cb.refData);

		state_mutex.unlock ();

		DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.informAll: invoking callback, sbn: ").print (sbn).pendl (); )
		if (informCallback != NULL) {
		    try {
			informCallback (sbn->cb.callback,
					sbn->cb.callbackData,
					data);
		    } catch (Exception &exc) {
			errf->print ((UintPtr) this).print (" MyCpp.GenericInformer.informAll: unexpected exception: ");
			printException (errf, exc);
			abortIfReached ();
		    }
		}
		DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.informAll: callback returned, sbn: ").print (sbn).pendl (); )

		if (sbn->cb.unrefCallback != NULL)
		    sbn->cb.unrefCallback (sbn->cb.refData);

		// It is a good idea to nullify the reference while state_mutex
		// is unlocked, because we're not utilizing destruction deferral
		// mechanisms of DataMutex in this case.
		obj = NULL;

		state_mutex.lock ();
	    }
	}

	sbnEl = sbnEl->next;
    }

    traversing --;
    if (traversing == 0) {
	sbnEl = subscriptions.first;
	while (sbnEl != NULL) {
	    nextSbnEl = sbnEl->next;
	    Ref<Subscription> const &sbn = sbnEl->data;

	    if (sbn->valid == false) {
		DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.informAll: post-processing invalid subscription, sbn: ").print (sbn).pendl (); )
		cleanupSubscription (sbn);
		subscriptions.remove (sbnEl);
	    } else {
		if (sbn->suspended) {
		    subscriptions.remove (sbnEl);
		    suspendedSbns.append (sbn, suspendedSbns.last);
		    sbn->sbnsLink = suspendedSbns.last;
		    DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.informAll: inSuspendedList = true").pendl (); )
		    sbn->inSuspendedList = true;
		}
	    }

	    sbnEl = nextSbnEl;
	}
    }

    state_mutex.unlock ();
}

GenericInformer::GenericInformer (InformCallback informCallback)
{
    this->informCallback = informCallback;
    traversing = 0;
}

GenericInformer::~GenericInformer ()
{
    DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.~()").pendl (); )

  DataMutexLock state_lock (state_mutex);

    {
	List< Ref<Subscription> >::DataIterator sbn_iter (subscriptions);
	while (!sbn_iter.done ()) {
	    Ref<Subscription> &sbn = sbn_iter.next ();
	    DEBUG ( errf->print (this).print (" MyCpp.GenericInformer.~(): sbn: ").print (sbn).pendl (); )
	    if (!sbn->valid)
		abortIfReached ();

	    cleanupSubscription (sbn);
	}
    }

    {
	List< Ref<Subscription> >::DataIterator sbn_iter (suspendedSbns);
	while (!sbn_iter.done ()) {
	    Ref<Subscription> &sbn = sbn_iter.next ();
	    if (!sbn->valid)
		abortIfReached ();

	    cleanupSubscription (sbn);
	}
    }
}

}

