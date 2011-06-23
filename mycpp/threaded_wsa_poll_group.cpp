#ifndef PLATFORM_WIN32
#error
#endif

#include <windows.h>
#include <winsock2.h>

#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/array_holder.h>

#include <mycpp/threaded_wsa_poll_group.h>

#define DEBUG(a) ;

using namespace MyCpp;

namespace MyCpp {

static void
requestPollableEvents (Pollable        *pollable,
		       IoActor::Events  events,
		       WSAEVENT         wsa_event)
    throw (InternalException)
{
    long wsa_events = 0;

    if (events & IoActor::EventRead)
	wsa_events |= FD_READ | FD_ACCEPT | FD_CLOSE;
    if (events & IoActor::EventWrite)
	wsa_events |= FD_WRITE | FD_OOB | FD_CONNECT | FD_CLOSE;
    if (events & IoActor::EventError)
	wsa_events |= FD_CLOSE;

    int rv = WSAEventSelect (pollable->getFd (), wsa_event, wsa_events);
    if (rv != 0) {
	int error = WSAGetLastError ();

	// This may fail, but we don't care already.
	WSACloseEvent (wsa_event);

	if (rv != SOCKET_ERROR)
	    throw InternalException ();

	throw InternalException (win32ErrorToString (error));
    }
}

void
ThreadedWsaPollGroup::need_input_callback (bool  need,
					   void *_pr)
{
    if (!need)
	return;

    PollableRecord *pr = static_cast <PollableRecord*> (_pr);
    Ref<ThreadedWsaPollGroup> self = pr->weak_threaded_wsa_poll_group.getRef ();
    if (self.isNull ())
	return;

  DataMutexLock state_lock (self->state_mutex);

    if (!pr->valid)
	return;

    Ref<Pollable> pollable = pr->weak_pollable.getRef ();
    if (pollable.isNull ())
	return;

    pr->events |= IoActor::EventRead;

    try {
	requestPollableEvents (pollable, pr->events, pr->wsa_event);
    } catch (Exception &exc) {
	errf->print ("MyCpp.ThreadedWsaPollGroup.need_input_callback: ");
	printException (errf, exc);
    }
}

void
ThreadedWsaPollGroup::need_output_callback (bool  need,
					    void *_pr)
{
    if (!need)
	return;

    PollableRecord *pr = static_cast <PollableRecord*> (_pr);
    Ref<ThreadedWsaPollGroup> self = pr->weak_threaded_wsa_poll_group.getRef ();
    if (self.isNull ())
	return;

  DataMutexLock state_lock (self->state_mutex);

    if (!pr->valid)
	return;

    Ref<Pollable> pollable = pr->weak_pollable.getRef ();
    if (pollable.isNull ())
	return;

    pr->events |= IoActor::EventWrite;

    try {
	requestPollableEvents (pollable, pr->events, pr->wsa_event);
    } catch (Exception &exc) {
	errf->print ("MyCpp.ThreadedWsaPollGroup.need_output_callback: ");
	printException (errf, exc);
    }
}

void
ThreadedWsaPollGroup::pollable_pre_closed_callback (void *_pr)
{
    PollableRecord *pr = static_cast <PollableRecord*> (_pr);
    Ref<ThreadedWsaPollGroup> self = pr->weak_threaded_wsa_poll_group.getRef ();
    if (self.isNull ())
	return;

    self->removePollable (pr);
}

void
ThreadedWsaPollGroup::pollable_deletion_callback (void *_pr)
{
    PollableRecord *pr = static_cast <PollableRecord*> (_pr);
    Ref<ThreadedWsaPollGroup> self = pr->weak_threaded_wsa_poll_group.getRef ();
    if (self.isNull ())
	return;

    self->removePollable (pr);
}

void
ThreadedWsaPollGroup::releasePollableRecord (PollableRecord *pr)
    throw (InternalException)
{
    abortIf (pr == NULL);

    if (!pr->valid)
	return;
    pr->valid = false;

    {
	Ref<Pollable> pollable = pr->weak_pollable.getRef ();
	if (!pollable.isNull ()) {
	    pollable->getNeedInputInformer ()->unsubscribe (pr->input_sbn);
	    pollable->getNeedOutputInformer ()->unsubscribe (pr->output_sbn);
	    pollable->getPreClosedInformer ()->unsubscribe (pr->pre_closed_sbn);
	    pollable->removeDeletionCallback (pr->del_sbn);
	}
    }

    if (!WSACloseEvent (pr->wsa_event))
	throw InternalException (win32ErrorToString (WSAGetLastError ()));
}

Ref<PollGroup::PollableRecord>
ThreadedWsaPollGroup::addPollable (Pollable *pollable,
				   IoActor::Events events)
    throw (InternalException)
{
    if (pollable == NULL)
	return NULL;

    Ref<PollableRecord> pr = grab (new PollableRecord);
    pr->valid = true;
    pr->weak_threaded_wsa_poll_group = this;
    pr->weak_pollable = pollable;
    pr->events = events;

    pr->wsa_event = WSACreateEvent ();
    if (pr->wsa_event == WSA_INVALID_EVENT)
	throw InternalException (win32ErrorToString (WSAGetLastError ()));

    try {
	requestPollableEvents (pollable, pr->events, pr->wsa_event);
    } catch (Exception &exc) {
	WSACloseEvent (pr->wsa_event);
	throw InternalException (String::nullString (), exc.clone ());
    }

  DataMutexLock state_lock (state_mutex);

    {
	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = this;
	cb.callback = need_input_callback;
	cb.callbackData = pr;
	cb.addRefData (pr);

	pr->input_sbn = pollable->getNeedInputInformer ()->subscribe (cb);
    }

    {
	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = this;
	cb.callback = need_output_callback;
	cb.callbackData = pr;
	cb.addRefData (pr);

	pr->output_sbn = pollable->getNeedOutputInformer ()->subscribe (cb);
    }

    {
	CallbackDesc<IoActor_Closing::PreClosedCallback> cb;
	cb.weak_obj = this;
	cb.callback = pollable_pre_closed_callback;
	cb.callbackData = pr;
	cb.addRefData (pr);

	if (pollable->isPreClosed_subscribe (&cb,
					     GenericInformer::InformOneshot,
					     &pr->pre_closed_sbn))
	{
	    releasePollableRecord (pr);
	    return NULL;
	}
    }

    {
	CallbackDesc<DeletionCallback> cb;
	cb.weak_obj = this;
	cb.callback = pollable_deletion_callback;
	cb.callbackData = pr;
	cb.addRefData (pr);

	pr->del_sbn = pollable->addDeletionCallbackNonmutual (cb);
    }

    pr->pollables_link = pollables.append (pr);
    num_pollables ++;

    return pr.ptr ();
}

void
ThreadedWsaPollGroup::removePollable (PollGroup::PollableRecord *_pr)
    // TODO throw (InternalException)
{
    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);

    if (!WSASetEvent (trigger_event))
	throw InternalException (win32ErrorToString (WSAGetLastError ()));

  MutexLock wait_lock (wait_mutex);

    if (!WSAResetEvent (trigger_event))
	throw InternalException (win32ErrorToString (WSAGetLastError ()));

    {
      DataMutexLock state_lock (state_mutex);

	DEBUG (
	    errf->print ("MyCpp.ThreadedWsaPollGroup.removePollable: removing").pendl ();
	)

	if (!pr->valid)
	    return;
	pollables.remove (pr->pollables_link);
	abortIf (num_pollables < 1);
	num_pollables --;
	releasePollableRecord (pr);
    }
}

Ref<Pollable>
ThreadedWsaPollGroup::poll (IoActor::Events *ret_events)
    throw (InternalException)
{
    if (ret_events != NULL)
	*ret_events = 0;

  MutexLock wait_lock (wait_mutex);

    {
      DataMutexLock state_lock (state_mutex);

	DEBUG (
	    errf->print ("MyCpp.ThreadedWsaPollGroup.poll: collecting data").pendl ();
	)

	abortIf (num_pollables != pollables.getNumElements ());
	DWORD num_wsa_events = num_pollables + 1;
	ArrayHolder< Ref<PollableRecord> > tmp_pollables (num_wsa_events - 1);
	ArrayHolder<WSAEVENT> wsa_events (num_wsa_events);

	Size i = 0;
	List< Ref<PollableRecord> >::DataIterator pr_iter (pollables);
	while (!pr_iter.done ()) {
	    Ref<PollableRecord> &pr = pr_iter.next ();
	    pr->events = 0;
	    tmp_pollables [i] = pr;
	    wsa_events [i] = pr->wsa_event;
	    i++;
	}
	wsa_events [i] = trigger_event;

	state_lock.unlock ();

	// FIXME limit num_wsa_events

	DWORD rv = WSAWaitForMultipleEvents (num_wsa_events,
					     wsa_events,
					     FALSE,        // fWaitAll
					     WSA_INFINITE, // dwTimeout
					     FALSE);       // fAlertable
	wait_lock.unlock ();

//	DEBUG (
//	    errf->print ("MyCpp.WsaPollGroup.poll: WSAWaitForMultipleEvents has returned").pendl ();
//	)
	if (rv == WSA_WAIT_FAILED) {
	    DEBUG (
		errf->print ("MyCpp.ThreadedWsaPollGroup.poll: WSA_WAIT_FAILED").pendl ();
	    )
	    throw InternalException (win32ErrorToString (WSAGetLastError ()));
	}

	if (! (rv >= WSA_WAIT_EVENT_0 && rv < WSA_WAIT_EVENT_0 + num_wsa_events) ) {
	    DEBUG (
		errf->print ("MyCpp.ThreadedWsaPollGroup.poll: unexpected rv").pendl ();
	    )
	    throw InternalException ();
	}

	DWORD ev_index = rv - WSA_WAIT_EVENT_0;
	abortIf (ev_index >= num_wsa_events);

	for (i = ev_index; i < num_wsa_events - 1; i++) {
	    {
	      DataMutexLock state_lock (state_mutex);

		if (tmp_pollables [i]->valid) {
		  if (!WSAResetEvent (wsa_events [i]))
		      throw InternalException (win32ErrorToString (WSAGetLastError ()));

		    Ref<Pollable> pollable = tmp_pollables [i]->weak_pollable.getRef ();
		    if (!pollable.isNull ()) {
			requestPollableEvents (pollable, 0, wsa_events [i]);

			// TEST
			state_lock.unlock ();
			pollable->processEvents (IoActor::EventRead | IoActor::EventWrite);
		    }
		}
	    }
	}

//	if (!WSAResetEvent (trigger_event))
//	    throw InternalException (win32ErrorToString (WSAGetLastError ()));
    }

    // FIXME Change the API
    return NULL;
}

void
ThreadedWsaPollGroup::trigger ()
// FIXME    throw (InternalException)
{
    if (!WSASetEvent (trigger_event))
	throw InternalException (win32ErrorToString (WSAGetLastError ()));

    wait_mutex.lock ();
    wait_mutex.unlock ();

    if (!WSAResetEvent (trigger_event))
	throw InternalException (win32ErrorToString (WSAGetLastError ()));
}

ThreadedWsaPollGroup::ThreadedWsaPollGroup ()
    throw (InternalException)
{
    DEBUG (
	errf->print ("MyCpp.ThreadedWsaPollGroup.()").pendl ();
    )

    num_pollables = 0;

#if 0
    trigger_event = CreateEvent (NULL,  // lpEventAttributes
				 FALSE, // bManualReset
				 FALSE, // bInitialState
				 NULL); // lpName
    if (trigger_event == NULL)
	throw InternalException (win32ErrorToString (GetLastError ()));
#endif

    trigger_event = WSACreateEvent ();
    if (trigger_event == WSA_INVALID_EVENT)
	throw InternalException (win32ErrorToString (WSAGetLastError ()));
}

ThreadedWsaPollGroup::~ThreadedWsaPollGroup ()
    throw (InternalException)
{
    if (!WSACloseEvent (trigger_event))
	throw InternalException (win32ErrorToString (WSAGetLastError ()));

    List< Ref<PollableRecord> >::DataIterator pr_iter (pollables);
    while (!pr_iter.done ()) {
	Ref<PollableRecord> &pr = pr_iter.next ();
	releasePollableRecord (pr);
    }
}

}

