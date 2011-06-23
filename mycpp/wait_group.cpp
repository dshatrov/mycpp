#include <mycpp/util.h>

#include <mycpp/wait_group.h>

namespace MyCpp {

void
WaitGroup::activeEvent (void *_wr)
{
    WaitableRecord * const &wr = static_cast <WaitableRecord*> (_wr);
    WaitGroup *self = wr->waitGroup;

    self->activeMutex.lock ();

    if (wr->state != WaitableRecord::ActiveState) {
	if (wr->state == WaitableRecord::InactiveState)
	    self->inactiveList.remove (wr->wgLink);
	else
	if (wr->state != WaitableRecord::UnknownState)
	    abortIfReached ();

	wr->state = WaitableRecord::ActiveState;
	wr->wgLink = self->activeList.append (wr);
	if (self->activeList.first == self->activeList.last)
	    self->activeCond.signal ();
    }

    self->activeMutex.unlock ();
}

Ref<WaitGroup::WaitableRecord>
WaitGroup::wait ()
{
    Ref<WaitableRecord> wr;

    activeMutex.lock ();

    while (activeList.first != NULL)
	activeCond.wait (activeMutex);

    wr = activeList.first->data;
    activeList.remove (activeList.first);
    wr->wgLink = inactiveList.append (wr);
    wr->state = WaitableRecord::InactiveState;

    activeMutex.unlock ();

    return wr;
}

Ref<WaitGroup::WaitableRecord>
WaitGroup::add (Waitable         *waitable,
		void             *userData,
		SimplyReferenced *ref_data)
{
    Ref<WaitableRecord> wr = grab (new WaitableRecord);
    wr->valid = true;
    // TODO Subscribe for deletion of 'waitable'.
    wr->weak_waitable = waitable;
    wr->userData = userData;
    wr->ref_data = ref_data;
    wr->state = WaitableRecord::UnknownState;

    {
	CallbackDesc<Waitable::ActiveCallback> cb;
	cb.weak_obj = this;
	cb.callback = activeEvent;
	cb.callbackData = wr;
	cb.addRefData (wr);

	wr->activeSbn = waitable->getActiveInformer ()->subscribe (cb);
    }

    bool active = waitable->isActive ();

    activeMutex.lock ();

    if (!wr->valid)
	abortIfReached ();

    if (wr->state == WaitableRecord::UnknownState) {
	if (active) {
	    wr->state = WaitableRecord::ActiveState;
	    wr->wgLink = activeList.append (wr, activeList.last);
	    if (activeList.first == activeList.last)
		activeCond.signal ();
	} else {
	    wr->state = WaitableRecord::InactiveState;
	    wr->wgLink = inactiveList.append (wr, inactiveList.last);
	}
    }

    activeMutex.unlock ();

    return wr;
}

void
WaitGroup::remove (WaitableRecord *wr)
{
    activeMutex.lock ();

    if (wr->valid) {
	{
	    Ref<Waitable> waitable = wr->weak_waitable.getRef ();
	    if (!waitable.isNull ())
		waitable->getActiveInformer ()->unsubscribe (wr->activeSbn);
	}
	wr->weak_waitable = NULL;
	wr->activeSbn = NULL;
	// 'wr->ref_data' should NOT be cleared here, because the user may
	// still want to use this WaitableRecord object and call its
	// getUserData() method.

	if (wr->state == WaitableRecord::ActiveState)
	    activeList.remove (wr->wgLink);
	else
	if (wr->state == WaitableRecord::InactiveState)
	    inactiveList.remove (wr->wgLink);

	wr->valid = false;
    }

    activeMutex.unlock ();
}

WaitGroup::WaitGroup ()
{
    // Locking 'activeMutex' here is not strictly required, since
    // no method of class WaitableRecord locks 'activeMutex', and
    // WaitableRecord objects are the only traces of the WaitGroup
    // after its destruction.
    activeMutex.lock ();

    {
	List< Ref<WaitableRecord> >::DataIterator active_iter (activeList);
	while (!active_iter.done ()) {
	    Ref<WaitableRecord> wr = active_iter.next ();
	    {
		Ref<Waitable> waitable = wr->weak_waitable.getRef ();
		if (!waitable.isNull ())
		    waitable->getActiveInformer ()->unsubscribe (wr->activeSbn);
	    }
	    wr->weak_waitable = NULL;
	    wr->activeSbn = NULL;
	    wr->valid = false;
	}
	activeList.clear ();
    }

    {
	List< Ref<WaitableRecord> >::DataIterator inactive_iter (inactiveList);
	while (!inactive_iter.done ()) {
	    Ref<WaitableRecord> wr = inactive_iter.next ();
	    {
		Ref<Waitable> waitable = wr->weak_waitable.getRef ();
		if (!waitable.isNull ())
		    waitable->getActiveInformer ()->unsubscribe (wr->activeSbn);
	    }
	    wr->weak_waitable = NULL;
	    wr->activeSbn = NULL;
	    wr->valid = false;
	}
	inactiveList.clear ();
    }

    activeMutex.unlock ();
}

}

