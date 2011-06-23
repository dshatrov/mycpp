#include <mycpp/base_util.h>
#include <mycpp/mycpp_thread_local.h>
#include <mycpp/deletion_queue.h>

#include <mycpp/referenced.h>

#define DEBUG(a) ;

namespace MyCpp {

bool
Referenced::sub_last_unref ()
{
    int old;

#if 0
NO DISPOSE
    _dispose ();
#endif

    /* Might have been re-referenced in _dispose() or
     * via a weak reference. */
    for (;;) {
	old = refCount.get ();
	if (old > 1) {
	    if (!refCount.compareAndExchange (old, old - 1))
		continue;

	    return false;
	}

	break;
    }

    stateMutex.lock ();
    if (shadow.isNull ()) {
	/* We're still removing the last reference, so
	 * if we determine that no shadow has been created so far,
	 * then that means that there is _no_ weak references
	 * to the object. */
	stateMutex.unlock ();

	{
	    MyCpp_ThreadLocal *tlocal = myCpp_getThreadLocal ();
	    if (tlocal->data_mutex_counter > 0) {
		// Is it possible that this will be called on and on
		// if there is contention between dispose() and WeakRef.getRef()
		// in a different thread or in one of the dispose handlers?
		//
		// UPD 08.07.07: This was the reason why the whole dispose()
		// technique was abolished.
		//
		// Note 09.09.07: This works because deletion queues are thread-local.
		deletionQueue_append (this);
		refCount.add (-1);
		return false;
	    }
	}

      // FIXME We presume that if an object does not have a shadow, then there
      // is no deletion subscriptions to process. This must be false at least
      // for nonmutual deletion callbacks, hence we should invoke deletion
      // callbacks here.

      /* It is safe to call 'delete this' at this point. */
    } else {
	stateMutex.unlock ();

	shadow->rw_lock.writerLock ();

	/* Now, it is guaranteed that no references will be added
	 * to the object by any means except by using weak references.
	 * A reference could have been added before we've aquired
	 * shadow->rw_lock "for writing". */
	for (;;) {
	    old = refCount.get ();
	    if (old > 1) {
		if (!refCount.compareAndExchange (old, old - 1))
		    continue;

		shadow->rw_lock.writerUnlock ();
		return false;
	    }

	    break;
	}

	shadow->weak_ptr = NULL;

	// TEST (comment out after test)
	shadow->rw_lock.writerUnlock ();

	/* mutualDeletionCallback() will _not_be called at this point,
	 * thanks to WeakRef's on this object.
	 * removeDeletionCallback() will not be called for the same reason.
	 * Hence no need to use 'deletionMutex' for synchronization. */
	List< Ref<DeletionSubscription> >::DataIterator sbn_iter (deletion_subscriptions);
	List< Ref<DeletionSubscription> >::DataIterator sbn_iter_start = sbn_iter;

	List< Ref<Referenced> > objs;

      // FIXME Calling deletion callbacks with a DataMutex seems to be
      // incorrect. We should defer invocation of deletion callbacks until
      // all data mutexes are relesed.
      // NOTE: They _are_ relased when we call deletion callbacks, but we're
      // doing unnecessary copying of deletion_subscriptions list contents
      // to 'objs' here.

	/* NOTE: "refCallback"s must be called _before releasing rw_lock
	 * to avoid a nasty race in potential client code.
	 *
	 * UPD 08.07.07: I wish I had described that race back then. */
	while (!sbn_iter.done ()) {
	    Ref<DeletionSubscription> &sbn = sbn_iter.next ();
	  DEBUG (
	    if (traced.get ())
		fprintf (stderr, "--- traced: invoking ref\n");
	  ) /* (DEBUG) */

	    if (sbn->cb.weak_obj.isValid ()) {
	      // DEBUG
//		abortIf (sbn->cb.weak_obj.getShadow () == shadow);
	      // (DEBUG)
		Ref<Referenced> obj;
		obj = sbn->cb.weak_obj.getRef ();
		if (!obj.isNull ()) {
		    objs.append (obj);
		} else {
		    /* 'sbn' is not removed from 'deletion_subscriptions' list deliberately,
		     * because it is scary to trigger nullification of 'sbn->cb.rdata_list'
		     * with 'shadow->rw_lock' held. */
		    sbn->invalid.set (1);
		    continue;
		}
	    }

	    if (sbn->cb.refCallback != NULL)
		sbn->cb.refCallback (sbn->cb.refData);
	}

	// TEST (uncomment)
//	shadow->rw_lock.writerUnlock ();

	{
	    MyCpp_ThreadLocal *tlocal = myCpp_getThreadLocal ();
	    if (tlocal->data_mutex_counter > 0) {
		// Is it possible that this will be called on and on
		// if there is contention between dispose() and WeakRef.getRef()
		// in a different thread or in one of the dispose handlers?
		//
		// UPD 08.07.07: This was the reason why the whole dispose()
		// technique was abolished.
		deletionQueue_append (this);
		refCount.add (-1);
		return false;
	    }
	}

	/* NOTE! It is important not to defer invocation of the callbacks
	 * (like invoking them in the destructor), because it may trigger
	 * bugs due to unexpected deinitializatoin order (think of objects
	 * subscribed to deletion of their members) */

	sbn_iter = sbn_iter_start;
	while (!sbn_iter.done ()) {
	    Ref<DeletionSubscription> &sbn = sbn_iter.next ();
	    if (sbn->invalid.get ())
		continue;

	    /* TODO Нет ли здесь гонки?
	     * Проверка sbn->invalid выглядит неубедительно. */

	    if (sbn->cb.callback != NULL)
		sbn->cb.callback (sbn->cb.callbackData);

	  DEBUG (
	    if (traced.get ())
		fprintf (stderr, "--- ~Referenced(): checking unref\n");
	  ) /* (DEBUG) */
	    if (sbn->cb.unrefCallback != NULL) {
	      DEBUG (
		if (traced.get ())
		    fprintf (stderr, "--- ~Referenced(): invoking unref\n");
	      ) /* (DEBUG) */
		sbn->cb.unrefCallback (sbn->cb.refData);
	    }

	    if (!sbn->mutual_sbn.isNull ()) {
		/* getRef() has already been called at this point,
		 * so this is an ambiguous extra call. This could be improved,
		 * e.g. by iterating through 'objs' in parallel with 'sbn_iter'. */
		Ref<Referenced> obj;
		obj = sbn->cb.weak_obj.getRef ();
		if (!obj.isNull ())
		    obj->removeDeletionCallback (sbn->mutual_sbn);

		sbn->mutual_sbn = NULL;
		sbn->cb.reset ();
	    }
	}
	deletion_subscriptions.clear ();

	/* Note that lifetime of 'objs' ends here. */

	/* It is safe to call 'delete this' at this point. */
    }

    return true;
}

void
Referenced::mutualDeletionCallback (void *_sbn)
{
    DeletionSubscription *sbn = static_cast <DeletionSubscription*> (_sbn);
    Referenced *&self = sbn->this_obj;

    if (!sbn->invalid.compareAndExchange (0, 1))
	return;

  DEBUG (
    if (self->traced.get ())
	fprintf (stderr, "--- traced: mutualDeletionCallback()\n");
  ) /* (DEBUG) */

    self->deletionMutex.lock ();
    self->deletion_subscriptions.remove (sbn->obj_link);
    self->deletionMutex.unlock ();

    sbn->mutual_sbn = NULL;
    sbn->cb.reset ();
}

Ref<Referenced::DeletionSubscription>
Referenced::addDeletionCallback (const CallbackDesc<DeletionCallback> &cb)
{
    Ref<Referenced> obj;
    if (cb.weak_obj.isValid ()) {
	obj = cb.weak_obj.getRef ();
	if (obj.isNull ())
	    return NULL;
    }

    Ref<DeletionSubscription> sbn = grab (new DeletionSubscription);
    sbn->cb = cb;
    sbn->this_obj = this;

    {
      MutexLock deletion_lock (deletionMutex);

	if (!obj.isNull ()) {
	    CallbackDesc<DeletionCallback> mcb;
	    mcb.weak_obj = this;
	    mcb.callback = mutualDeletionCallback;
	    mcb.callbackData = static_cast <void*> (sbn);
	    mcb.addRefData (sbn);
	    sbn->mutual_sbn = obj->addDeletionCallbackNonmutual (mcb);
	}

	sbn->obj_link = deletion_subscriptions.append (sbn);
    }

    return sbn;
}

Ref<Referenced_Shadow>
Referenced::getShadow ()
{
    stateMutex.lock ();

    if (shadow.isNull ()) {
	shadow = grab (new Referenced_Shadow ());
	shadow->weak_ptr = this;
    }

    stateMutex.unlock ();

    return shadow;
}

Ref<Referenced::DeletionSubscription>
Referenced::addDeletionCallbackNonmutual (const CallbackDesc<DeletionCallback> &cb)
{
    Ref<DeletionSubscription> sbn = grab (new DeletionSubscription);
    sbn->cb = cb;
    sbn->this_obj = this;

    deletionMutex.lock ();
    sbn->obj_link = deletion_subscriptions.append (sbn);
    deletionMutex.unlock ();

    return sbn;
}

void
Referenced::removeDeletionCallback (DeletionSubscription *sbn)
{
    if (sbn == NULL)
	return;

    if (!sbn->invalid.compareAndExchange (0, 1))
	return;

  DEBUG (
    if (traced.get ())
	fprintf (stderr, "--- traced: removeDeletionCallback()\n");
  ) /* (DEBUG) */

    deletionMutex.lock ();
    deletion_subscriptions.remove (sbn->obj_link);
    deletionMutex.unlock ();

    if (!sbn->mutual_sbn.isNull ()) {
	Ref<Referenced> obj;
	obj = sbn->cb.weak_obj.getRef ();
	if (!obj.isNull ())
	    obj->removeDeletionCallback (sbn->mutual_sbn);
    }

    sbn->mutual_sbn = NULL;
    sbn->cb.reset ();
}

Referenced::~Referenced ()
{
  DEBUG (
    if (traced.get ())
	fprintf (stderr, "--- traced: ~Referenced()\n");
  ) /* (DEBUG) */
}

}

