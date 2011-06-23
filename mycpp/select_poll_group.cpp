#include <errno.h>
#include <unistd.h>
#include <sys/select.h>

#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/select_poll_group.h>

// Flow
#define DEBUG_FLO(a)
// Buzz
#define DEBUG_BUZ(a)
#define DEBUG(a) a

namespace MyCpp {

void
SelectPollGroup::needInputEvent (bool  need,
				 void *_pr)
{
    PollableRecord *pr = static_cast <PollableRecord*> (_pr);
    Ref<SelectPollGroup> self = pr->weak_group.getRef ();
    if (self.isNull ())
	return;

  DEBUG (
    errf->print ((UintPtr) self.ptr()).print (" MyCpp.SelectPollGroup.needInputEvent: ").print ((UintPtr) pr->pollable.ptr()).pendl ();
  )

  // TODO if needInputEvent() is called from a different thread than
  // the current poll() iteration, then we should do trigger() here.

    self->pollablesMutex.lock ();
    if (pr->valid)
	pr->needInput = need;
    self->pollablesMutex.unlock ();
}

void
SelectPollGroup::needOutputEvent (bool  need,
				  void *_pr)
{
    PollableRecord *pr = static_cast <PollableRecord*> (_pr);
    Ref<SelectPollGroup> self = pr->weak_group.getRef ();
    if (self.isNull ())
	return;

  DEBUG (
    errf->print ((UintPtr) self.ptr()).print (" MyCpp.SelectPollGroup.needOutputEvent: ").print ((UintPtr) pr->pollable.ptr()).pendl ();
  )

  // TODO if needInputEvent() is called from a different thread than
  // the current poll() iteration, then we should do trigger() here.

    self->pollablesMutex.lock ();
    if (pr->valid)
	pr->needOutput = need;
    self->pollablesMutex.unlock ();
}

Ref<PollGroup::PollableRecord>
SelectPollGroup::addPollable (Pollable        *pollable,
			      /* FIXME Why is this unused? */
			      IoActor::Events  events)
    throw (InternalException)
{
    if (pollable == NULL)
	return NULL;

    DEBUG (
	errf->print ((UintPtr) this).print (" MyCpp.SelectPollGroup.addPollable: ").print ((UintPtr) pollable).pendl ();
    )

    pollablesMutex.lock ();

    Ref<PollableRecord> pr = grab (new PollableRecord);
    pr->pollablesLink = pollables.appendEmpty ();
    pr->pollablesLink->data = pr;
    pr->activePollablesLink = NULL;
    pr->pollable = pollable;
    pr->weak_group = this;
    pr->valid = true;
    pr->needInput  = true;
    pr->needOutput = true;
    pr->pendingEvents = 0;

    {
	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = pr;
	cb.callback = needInputEvent;
	cb.callbackData = pr;

	pr->needInputSbn = pollable->getNeedInputInformer ()->subscribe (cb);
    }

    {
	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = pr;
	cb.callback = needOutputEvent;
	cb.callbackData = pr;

	pr->needOutputSbn = pollable->getNeedOutputInformer ()->subscribe (cb);
    }

    pollablesMutex.unlock ();

    /* Make the fd participate in a call to select() as soon as possible. */
    waitLock ();
    waitUnlock ();

    return pr.ptr ();
}

void
SelectPollGroup::invalidatePollable (PollableRecord *pr)
{
    if (!pr->valid)
	/* Paranoid check. */
	abortIfReached_str ("MyCpp.PollGroup.invalidatePollable: invalid Pollable");

    if (!pr->needInputSbn.isNull ()) {
	pr->pollable->getNeedInputInformer ()->unsubscribe (pr->needInputSbn);
	pr->needInputSbn = NULL;
    }

    if (!pr->needOutputSbn.isNull ()) {
	pr->pollable->getNeedOutputInformer ()->unsubscribe (pr->needOutputSbn);
	pr->needOutputSbn = NULL;
    }

    pr->valid = false;
    pr->pollable = NULL;
}

void
SelectPollGroup::removePollable (PollGroup::PollableRecord *_pr)
    throw (InternalException)
{
    PollableRecord *pr = static_cast <PollableRecord*> (_pr);

    if (pr == NULL)
	return;

    waitLock ();
    pollablesMutex.lock ();

    if (pr->valid) {
	invalidatePollable (pr);
	if (pr->activePollablesLink != NULL)
	    activePollables.remove (pr->activePollablesLink);
	pollables.remove (pr->pollablesLink);
    }

    pollablesMutex.unlock ();
    waitUnlock ();
}

Ref<SelectPollGroup::PollableRecord>
SelectPollGroup::dequeueActivePollable (IoActor::Events *events)
{
    while (activePollables.first != NULL) {
	List< Ref<PollableRecord> >::Element *pr_el = activePollables.first;
	Ref<PollableRecord> pr = pr_el->data;

	if (!pr->valid)
	    abortIfReached_str ("MyCpp.SelectPollGroup.getActivePollable: "
			    "invalid Pollable");

	if (!pr->pollable.isNull () &&
	    pr->pendingEvents != 0)
	{
	    if (events != NULL)
		*events = pr->pendingEvents;
	    pr->pendingEvents = 0;

	    pr->activePollablesLink = NULL;
	    activePollables.remove (pr_el);
	    return pr;
	} else {
	    pr->pendingEvents = 0;
	    pr->activePollablesLink = NULL;
	    activePollables.remove (pr_el);
	}
    }

    return NULL;
}

Ref<Pollable>
SelectPollGroup::poll (IoActor::Events * const events_ret,
		       int               const timeout)
    throw (InternalException)
{
    int nfds;
    Ref<PollableRecord> pr;

    Ref<Pollable> pollable;

    for (;;) {
	selectMutex.lock ();
	pollablesMutex.lock ();

	IoActor::Events events;

	pr = dequeueActivePollable (&events);
	if (!pr.isNull ()) {
	    if (!pr->valid)
		abortIfReached_str ("MyCpp.SelectPollGroup.poll: "
				    "invalid Pollable (after dequeueing)");

	    Ref<Pollable> pollable = pr->pollable;

	    if (events & IoActor::EventError) {
		invalidatePollable (pr);
		if (pr->activePollablesLink != NULL)
		    activePollables.remove (pr->activePollablesLink);
		pollables.remove (pr->pollablesLink);
	    }

	    if (!pollable.isNull () &&
		events != 0)
	    {
		pollablesMutex.unlock ();
		selectMutex.unlock ();

		if (events_ret != NULL)
		    *events_ret = events;

#if 0
		DEBUG (
		    errf->print (this).print (" MyCpp.SelectPollGroup.poll: returning ").print (pollable).pendl ();
		)
#endif
		return pollable;
	    }
	    /* else... */

	    pr = NULL;
	}

	fd_set rfds,
	       wfds,
	       efds;

	FD_ZERO (&rfds);
	FD_ZERO (&wfds);
	FD_ZERO (&efds);

	int largest_fd = triggerPipe [0];
	FD_SET (triggerPipe [0], &rfds);

	{
	    List< Ref<PollableRecord> >::DataIterator pr_iter (pollables);
	    while (!pr_iter.done ()) {
		Ref<PollableRecord> &pr = pr_iter.next ();

		if (!pr->valid)
		    abortIfReached_str ("MyCpp.SelectPollGroup: "
					"invalid Pollable (before select)\n");

		if (!pr->pollable.isNull ()) {
		    int fd = pr->pollable->getFd ();

		    if (fd > largest_fd)
			largest_fd = fd;

		    if (fd != -1) {
			DEBUG (
			    errf->print ((UintPtr) this).print (" MyCpp.SelectPollGroup: selecting ").print ((UintPtr) pr->pollable.ptr()).print (" for errors").pendl ();
			)
			FD_SET (fd, &efds);
			if (pr->needInput) {
			    FD_SET (fd, &rfds);
			    DEBUG (
				errf->print ((UintPtr) this).print (" MyCpp.SelectPollGroup: selecting ").print ((UintPtr) pr->pollable.ptr()).print (" for input").pendl ();
			    )
			}
			if (pr->needOutput) {
			    FD_SET (fd, &wfds);
			    DEBUG (
				errf->print ((UintPtr) this).print (" MyCpp.SelectPollGroup: selecting ").print ((UintPtr) pr->pollable.ptr()).print (" for output").pendl ();
			    )
			}
		    }
		}
	    }
	}

	pollablesMutex.unlock ();

	DEBUG (
	    errf->print ((UintPtr) this).print (" MyCpp.SelectPollGroup.poll: calling select()").pendl ();
	)

	{
	    struct timeval timeout_val;
	    timeout_val.tv_sec = timeout / 1000;
	    timeout_val.tv_usec = timeout % 1000 * 1000;

	    nfds = select (largest_fd + 1, &rfds, &wfds, &efds, timeout != -1 ? &timeout_val : NULL);
	}

	DEBUG (
	    errf->print ((UintPtr) this).print (" MyCpp.SelectPollGroup.poll: select() returned").pendl ();
	)

	if (nfds == -1) {
	    selectMutex.unlock ();
	    if (errno == EINTR)
		continue;

	    throw InternalException (
		    String::forPrintTask (
			(Pr ("MyCpp.SelectPollGroup.poll: "
			     "select failed: "))
			(Pr (errnoToString (errno)))));
	}

	if (nfds == 0) {
	    selectMutex.unlock ();

#if 0
// This may happen now
	    /* TEST */
	    abortIfReached_str ("MyCpp.SelectPollGroup: select returned \"0\""
				"(should not happen)");
#endif

	    return NULL;
	}

	pollablesMutex.lock ();

	{
	    List< Ref<PollableRecord> >::DataIterator pr_iter (pollables);
	    while (!pr_iter.done ()) {
		Ref<PollableRecord> &pr = pr_iter.next ();

		if (!pr->valid)
		    abortIfReached_str ("MyCpp.SelectPollGroup: "
					"invalid Pollable (after select)\n");

		if (!pr->pollable.isNull ()) {
		    int fd = pr->pollable->getFd ();

		    if (FD_ISSET (fd, &rfds)) {
			pr->needInput = false;
			pr->pendingEvents |= IoActor::EventRead;
		    }

		    if (FD_ISSET (fd, &wfds)) {
			pr->needOutput = false;
			pr->pendingEvents |= IoActor::EventWrite;
		    }

		    if (FD_ISSET (fd, &efds))
			pr->pendingEvents |= IoActor::EventError;

		    if (pr->pendingEvents != 0 &&
			pr->activePollablesLink == NULL)
		    {
			pr->activePollablesLink =
				activePollables.append (pr,
							activePollables.last);
		    }
		}
	    }
	}

	if (FD_ISSET (triggerPipe [0], &rfds)) {
	    pollablesMutex.unlock ();
	    selectMutex.unlock ();
	    return NULL;
	}

	pollablesMutex.unlock ();
	selectMutex.unlock ();
    }

    return NULL;
}

void
SelectPollGroup::waitLock ()
{
    int rv;
    char c;

    // FIXME What if there's no reader thread, and this write() syscall blocks?
    rv = write (triggerPipe [1], "A", 1);
    if (rv != 1)
	abortIfReached_str (
		String::forPrintTask (
		    (Pr ("MyCpp.PollGroup.trigger: write: "))
		    (Pr (errnoToString (errno))))->getData ());

    selectMutex.lock ();

    rv = read (triggerPipe [0], &c, 1);
    if (rv != 1)
	abortIfReached_str (
		String::forPrintTask (
		    (Pr ("MyCpp.PollGroup.trigger: read: "))
		    (Pr (errnoToString (errno))))->getData ());
}

void
SelectPollGroup::waitUnlock ()
{
    selectMutex.unlock ();
}

void
SelectPollGroup::trigger ()
    throw (InternalException)
{
    waitLock ();
    waitUnlock ();
}

SelectPollGroup::SelectPollGroup ()
    throw (InternalException)
{
    if (pipe (triggerPipe))
	throw InternalException (
		String::forPrintTask (
		    (Pr ("MyCpp::PollGroup(): "))
		    (Pr (errnoToString (errno)))));

}

SelectPollGroup::~SelectPollGroup ()
{
  DEBUG_BUZ (
    errf->print ("MyCpp.SelectPollGroup.~()")
	 .pendl ();
  );

    {
	pollablesMutex.lock ();

	{
	    List< Ref<PollableRecord> >::DataIterator pr_iter (pollables);
	    while (!pr_iter.done ())
		invalidatePollable (pr_iter.next ());
	}

	pollables.clear ();
	activePollables.clear ();

	pollablesMutex.unlock ();
    }

    close (triggerPipe [0]);
    close (triggerPipe [1]);
}

}

