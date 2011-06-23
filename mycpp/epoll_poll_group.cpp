#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/epoll_poll_group.h>

#define DEBUG(a) ;
#define DEBUG2(a) ;
#define DEBUG_POLL(a) ;

// TODO For the sake of simplicity, I've chosen oneshot epoll mode,
// which is obviously not the most effective method, since it involves
// additional epoll_ctl system calls. There is room for optimization
// here.

// TODO Can epoll*() system calls return EINTR? If they can, then
// the code should be updated to handle that.

namespace MyCpp {

void
EpollPollGroup::updatePollableEventMask (PollableRecord *pr)
{
    state_mutex.assertLocked ();

    Ref<Pollable> pollable = pr->weak_pollable.getRef ();
    if (pollable.isNull ())
	return;

    int fd = pollable->getFd ();
    if (fd == -1)
	return;

    struct epoll_event ev;
    zeroMemory (ev);
    ev.events = EPOLLONESHOT;
    ev.data.ptr = (void*) pr;

    if (pr->needInput)
	ev.events |= EPOLLIN;

    if (pr->needOutput)
	ev.events |= EPOLLOUT;

    // I don't see any reason for this epoll_ctl() to fail.
    if (epoll_ctl (efd, EPOLL_CTL_MOD, fd, &ev))
	errf->print ("MyCpp.EpollPollGroup.updatePollableEventMask: WARNING: "
		     "epoll_ctl failed: ").
	      print (errnoToString (errno)).
	      pendl ();
}

void
EpollPollGroup::needInputEvent (bool  need,
				void *_pr)
{
    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);
    Ref<EpollPollGroup> self = pr->weak_group.getRef ();
    if (self.isNull ())
	return;

    DEBUG (
	errf->print ("MyCpp.EpollPollGroup.needInputEvent: "
		     "pollable: ").print ((Uint64) pr).print (", "
		     "need: ").print (need ? "true" : "false").pendl ();
    )

    self->state_mutex.lock ();

    if (pr->valid &&
	pr->needInput != need)
    {
	pr->needInput = need;
	self->updatePollableEventMask (pr);
    }

    self->state_mutex.unlock ();
}

void
EpollPollGroup::needOutputEvent (bool  need,
				 void *_pr)
{
    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);
    Ref<EpollPollGroup> self = pr->weak_group.getRef ();
    if (self.isNull ())
	return;

    DEBUG (
	errf->print ("MyCpp.EpollPollGroup.needOutputEvent: "
		     "pollable: ").print ((Uint64) pr).print (", "
		     "need: ").print (need ? "true" : "false").pendl ();
    )

    self->state_mutex.lock ();

    if (pr->valid &&
	pr->needOutput != need)
    {
	pr->needOutput = need;
	self->updatePollableEventMask (pr);
    }

    self->state_mutex.unlock ();
}

void
EpollPollGroup::pollable_pre_closed_callback (void *_pr)
{
    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);
    Ref<EpollPollGroup> self = pr->weak_group.getRef ();
    if (self.isNull ())
	return;

    self->removePollable (pr);
}

void
EpollPollGroup::pollable_deletion_callback (void *_pr)
{
    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);
    Ref<EpollPollGroup> self = pr->weak_group.getRef ();
    if (self.isNull ())
	return;

    self->state_mutex.lock ();

    if (pr->valid) {
	// We shouldn't have any valid PollableRecords for deleted
	// pollables. All PollableRecord objects should be invalidated
	// in pollable_pre_closed_callback() at this point.
// TODO	abortIfReached ();
    }

    self->state_mutex.unlock ();
}

Ref<PollGroup::PollableRecord>
EpollPollGroup::addPollable (Pollable        *pollable,
			     IoActor::Events  events)
    throw (InternalException)
{
    DEBUG_POLL (
	errf->print (this).print (" MyCpp.EpollPollGroup.addPollable: ").print (pollable).pendl ();
    )

    if (pollable == NULL)
	return NULL;

    int fd = pollable->getFd ();
    if (fd == -1)
	return NULL;

    Ref<PollableRecord> pr = grab (new PollableRecord);
    pr->weak_pollable = pollable;
    pr->weak_group = this;
    pr->valid = true;
    pr->needInput  = events & IoActor::EventRead;
    pr->needOutput = events & IoActor::EventWrite;

    struct epoll_event ev;
    zeroMemory (ev);
    /* 07.05.26 I simply drop support for urgent data, since it's
     * TCP-specific, and there's no such concept in MyRelay I/O model. */
    ev.data.ptr = (void*) pr;
    ev.events = EPOLLONESHOT;
    if (events & IoActor::EventRead)
	ev.events |= EPOLLIN;
    if (events & IoActor::EventWrite)
	ev.events |= EPOLLOUT;
    // Errors are always reported, IoActor::EventError has no effect.

    {
      // Pollable will not be deleted till the end of this method,
      // hence we can subscribe for deletion without holding state_mutex.
      // (PollableRecord is not yet fully initialized here)

#if 0
	CallbackDesc<Object::DeletionCallback> cb;
	cb.weak_obj = this;
	cb.callback = pollable_deletion_callback;
	cb.callbackData = pr;
	cb.addRefData (pr);

	pr->pollable_deletion_sbn = pollable->addDeletionCallback (cb);
#endif
	pr->pollable_deletion_sbn = pollable->addDeletionCallbackNonmutual (pollable_deletion_callback, pr, pr, this);
    }

    state_mutex.lock ();
    pr->groupLink = pollables.append (pr);

    // Note that we must subscribe for PreClosed, NeedInput and NeedOutput events
    // with state_mutex held to be able to synchronise with invocations of
    // epoll*() system calls.

    {
	CallbackDesc<Pollable::PreClosedCallback> cb;
	cb.weak_obj = this;
	cb.callback = pollable_pre_closed_callback;
	cb.callbackData = pr;
	cb.addRefData (pr);

	if (pollable->isPreClosed_subscribe (&cb, GenericInformer::InformOneshot, &pr->pollable_pre_closed_sbn)) {
	    // If the Pollable is pre-closed then we shouldn't add it
	    // to the poll group at all. Note that invalidatePollable()
	    // should be able to cope with (a bit) partial initialization
	    // of the PollableRecord at this point.

	    invalidatePollable (pr);
	    return NULL;
	}
    }

    {
	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = this;
	cb.callback = needInputEvent;
	cb.callbackData = pr;
	cb.addRefData (pr);

	pr->needInputSbn = pollable->getNeedInputInformer ()->subscribe (cb);
    }

    {
	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = this;
	cb.callback = needOutputEvent;
	cb.callbackData = pr;
	cb.addRefData (pr);

	pr->needOutputSbn = pollable->getNeedOutputInformer ()->subscribe (cb);
    }

    int rv = epoll_ctl (efd, EPOLL_CTL_ADD, fd, &ev);
    if (rv != 0) {
	state_mutex.unlock ();
	throw InternalException (
		String::forPrintTask (
		    (Pr ("MyCpp.EpollPollGroup.addPollable: "))
		    (Pr (errnoToString (errno)))));
    }

    state_mutex.unlock ();

    return pr.ptr ();
}

void
EpollPollGroup::invalidatePollable (PollableRecord *pr)
{
    state_mutex.assertLocked ();

    if (!pr->valid)
	// Paranoid check.
	abortIfReached ();

    Ref<Pollable> pollable = pr->weak_pollable.getRef ();
    if (!pollable.isNull ()) {
	if (!pr->pollable_deletion_sbn.isNull ())
	    pollable->removeDeletionCallback (pr->pollable_deletion_sbn);

	if (!pr->pollable_pre_closed_sbn.isNull ())
	    pollable->getPreClosedInformer ()->unsubscribe (pr->pollable_pre_closed_sbn);

	if (!pr->needInputSbn.isNull ())
	    pollable->getNeedInputInformer ()->unsubscribe (pr->needInputSbn);

	if (!pr->needOutputSbn.isNull ())
	    pollable->getNeedOutputInformer ()->unsubscribe (pr->needOutputSbn);
    }

    pr->pollable_deletion_sbn = NULL;
    pr->pollable_pre_closed_sbn = NULL;
    pr->needInputSbn = NULL;
    pr->needOutputSbn = NULL;

    pr->valid = false;
    pr->weak_pollable = NULL;
    pr->weak_group = NULL;
}

void
EpollPollGroup::removePollable (PollGroup::PollableRecord *_pr)
    throw (InternalException)
{
    PollableRecord *pr = static_cast <PollableRecord*> (_pr);

    DEBUG_POLL (
    {
	Ref<Pollable> pollable = pr->weak_pollable.getRef ();
	errf->print (this).print (" MyCpp.EpollPollGroup.removePollable: ").print (pollable).pendl ();
    }
    )

    if (pr == NULL)
	return;

    waitLock ();
    state_mutex.lock ();

    if (!pr->valid) {
	state_mutex.unlock ();
	waitUnlock ();
	return;
    }

    Ref<Pollable> pollable = pr->weak_pollable.getRef ();
    int fd;
    if (!pollable.isNull ())
	fd = pollable->getFd ();
    else
	fd = -1;

    invalidatePollable (pr);
    pollables.remove (pr->groupLink);

    if (fd != -1) {
	if (epoll_ctl (efd, EPOLL_CTL_DEL, fd, NULL))
	    errf->print ("MyCpp.EpollPollGroup.removePollable: "
			 "epoll_ctl failed: ").
		  print (errnoToString (errno)).
		  pendl ();
    }

    state_mutex.unlock ();
    waitUnlock ();
}

Ref<Pollable>
EpollPollGroup::poll (IoActor::Events * const events,
		      int               const timeout)
    throw (InternalException)
{
    int nfds;
    Ref<PollableRecord> pr;

    struct epoll_event ev;
    zeroMemory (ev);

    for (;;) {
	// We lock/unlock wait_mutex to be sure that we don't skip
	// to epoll_wait after triggering on triggerPipe and before
	// waitLock() locks waitRwLock for writing.
	wait_mutex.lock ();
	wait_mutex.unlock ();

      DEBUG_POLL (
	state_mutex.lock ();
	List< Ref<PollableRecord> >::DataIterator pr_iter (pollables);
	errf->print (this).print (" MyCpp.EpollPollGroup.poll: pollables: ");
	while (!pr_iter.done ()) {
	    Ref<PollableRecord> &pr = pr_iter.next ();
	    Ref<Pollable> pollable = pr->weak_pollable.getRef ();
	    errf->print (pollable);
	    if (!pr_iter.done ())
		errf->print (", ");
	}
	errf->pendl ();
	state_mutex.unlock ();
      )

	waitRwLock.readerLock ();
	// TODO There is room for improvement, if we allow
	// acceptance of more than 1 event per epoll_wait call.
	nfds = epoll_wait (efd, &ev, 1 /* maxevents */, timeout);
	if (nfds == -1) {
	    waitRwLock.readerUnlock ();
	    if (errno == EINTR)
		continue;

	    throw InternalException (
		    String::forPrintTask (
			(Pr ("MyCpp.EpollPollGroup.poll: "))
			(Pr (errnoToString (errno)))));
	}

	if (nfds == 0) {
	    waitRwLock.readerUnlock ();
	    break;
	}

	if (nfds != 1) {
	    waitRwLock.readerUnlock ();
	    abortIfReached_str ("MyCpp.EpollPollGroup.poll: epoll_wait misbehaves");
	    continue;
	}

	pr = (PollableRecord*) ev.data.ptr;

	// Data from triggerPipe
	if (pr == NULL) {
	    waitRwLock.readerUnlock ();
	    break;
	}

	Ref<Pollable> pollable = pr->weak_pollable.getRef ();
	if (pollable.isNull ()) {
	    waitRwLock.readerUnlock ();
	    continue;
	}

	state_mutex.lock ();

	if (!pr->valid)
	    // Invalid PollableRecords should never leak into epoll_wait.
	    abortIfReached ();

	IoActor::Events evs = 0;

	bool update_event_mask = false;

	if (ev.events & EPOLLIN) {
	    pr->needInput = false;
	    update_event_mask = true;
	    evs |= IoActor::EventRead;
	}

	if (ev.events & EPOLLOUT) {
	    pr->needOutput = false;
	    update_event_mask = true;
	    evs |= IoActor::EventWrite;
	}

	if (ev.events & EPOLLPRI)
	    errf->print ("MyCpp.EpollPollGroup.poll: EPOLLPRI").pendl ();

	if (ev.events & EPOLLERR ||
	    ev.events & EPOLLHUP)
	{
	    DEBUG (
		if (ev.events & EPOLLERR)
		    errf->print ("EPOLLERR: ").
			  print ((unsigned long) ev.data.ptr).
			  pendl ();
		else
		    errf->print ("EPOLLHUP: ").
			  print ((unsigned long) ev.data.ptr).
			  pendl ();
	    )

	    evs |= IoActor::EventError;
	}

	DEBUG (
	    errf->print ("MyCpp.EpollPollGroup.poll: epoll_wait() returned: "
			 "pollable: ").print ((Uint64) pollable.ptr ()).print (", "
			 "events: ");
	    if (evs & IoActor::EventRead)
		errf->print ("Read ");
	    if (evs & IoActor::EventWrite)
		errf->print ("Write ");
	    if (evs & IoActor::EventError)
		errf->print ("Error ");
	    errf->pendl ();
	)

	if (update_event_mask)
	    updatePollableEventMask (pr);

	state_mutex.unlock ();

	// NOTE: It is important to interpret Pollable's pointer
	// and ref the Pollable before releasing reader lock.
	waitRwLock.readerUnlock ();

	if (evs & IoActor::EventError)
	    removePollable (pr);
	pr = NULL;

	if (events != NULL)
	    *events = evs;

	return pollable;
    }

    return NULL;
}

void
EpollPollGroup::waitLock ()
{
    int rv;

    // wait_mutex serves a double purpose here.
    // Firstly, it prevents busy-looping in poll() after data has been written
    // into the trigger pipe but waitRwLock has not been writer-locked yet.
    // Secondly, it allows for multiple simultaneous calls to waitLock(),
    // so that the trigger pipe doesn't get overloaded (only one byte
    // travels through it at a time).
    wait_mutex.lock ();

    // Note that triggerPipe[0] is polled with no EPOLLONESHOT flag set,
    // hence all active polls will wake up.
    rv = write (triggerPipe [1], "A", 1);
    // TODO This is a rough simplification: here we suppose that writing
    // a single byte into an empty pipe always writes the byte instantly
    // and never fails.
    if (rv != 1)
	abortIfReached_str (
		String::forPrintTask (
		    (Pr ("MyCpp.EpollPollGroup.trigger: write: "))
		    (Pr (errnoToString (errno))))->getData ());

    waitRwLock.writerLock ();

    char c;
    // TODO This yet another rough simplification: here we suppose that
    // reading a single byte from a non-empty pipe always reads the byte
    // instantly and never fails.
    rv = read (triggerPipe [0], &c, 1);
    if (rv != 1)
	abortIfReached_str (
		String::forPrintTask (
		    (Pr ("MyCpp.EpollPollGroup.trigger: read: "))
		    (Pr (errnoToString (errno))))->getData ());

    wait_mutex.unlock ();
}

void
EpollPollGroup::waitUnlock ()
{
    waitRwLock.writerUnlock ();
}

void
EpollPollGroup::trigger ()
    throw (InternalException)
{
    waitLock ();
    waitUnlock ();
}

void
EpollPollGroup::closeTriggerPipeOne ()
{
    for (;;) {
	int res = close (triggerPipe [0]);
	if (res == -1) {
	    if (res == EINTR)
		continue;

	    errf->print ("MyCpp.EpollPollGroup.closeTriggerPipeOne(): "
			 "WARNING: close() failed").pendl ();
	} else
	if (res != 0) {
	    errf->print ("MyCpp.EpollPollGroup.closeTriggerPipeOne(): "
			 "WARNING: unexpected return value from close()").pendl ();
	}

	break;
    }
}

void
EpollPollGroup::closeTriggerPipeTwo ()
{
    for (;;) {
	int res = close (triggerPipe [1]);
	if (res == -1) {
	    if (res == EINTR)
		continue;

	    errf->print ("MyCpp.EpollPollGroup.closeTriggerPipeTwo(): "
			 "WARNING: close() failed").pendl ();
	} else
	if (res != 0) {
	    errf->print ("MyCpp.EpollPollGroup.closeTriggerPipeTwo: "
			 "WARNING: unexpected return value from close()").pendl ();
	}

	break;
    }
}

void
EpollPollGroup::closeEfd ()
{
    for (;;) {
	int res = close (efd);
	if (res == -1) {
	    if (res == EINTR)
		continue;

	    errf->print ("MyCpp.EpollPollGroup.closeEfd: "
			 "WARNING: close() failed").pendl ();
	} else
	if (res != 0) {
	    errf->print ("MyCpp.EpollPollGroup.closeEfd: "
			 "WARNING: unexpected return value from close()").pendl ();
	}

	break;
    }
}

EpollPollGroup::EpollPollGroup ()
    throw (InternalException)
{
    DEBUG2 (
	errf->print (this).print (" MyCpp.EpollPollGroup.()").pendl ();
    )

    if (pipe (triggerPipe))
	throw InternalException (
		String::forPrintTask (
		    (Pr ("MyCpp::PollGroup(): "))
		    (Pr (errnoToString (errno)))));

    efd = epoll_create (1);
    if (efd == -1) {
	closeTriggerPipeOne ();
	closeTriggerPipeTwo ();
	throw InternalException (
		String::forPrintTask (
		    (Pr ("MyCpp::PollGroup(): "))
		    (Pr (errnoToString (errno)))));
    }

    struct epoll_event ev;
    zeroMemory (ev);
    ev.events = EPOLLIN;
    ev.data.ptr = NULL;

    int rv;

    rv = epoll_ctl (efd, EPOLL_CTL_ADD, triggerPipe [0], &ev);
    if (rv != 0) {
	closeTriggerPipeOne ();
	closeTriggerPipeTwo ();
	closeEfd ();
	throw InternalException (
		String::forPrintTask (
		    (Pr ("MyCpp::PollGroup(): "))
		    (Pr (errnoToString (errno)))));
    }
}

EpollPollGroup::~EpollPollGroup ()
{
    {
	state_mutex.lock ();

	{
	    List< Ref<PollableRecord> >::DataIterator pr_iter (pollables);
	    while (!pr_iter.done ())
		invalidatePollable (pr_iter.next ());
	}

	pollables.clear ();

	state_mutex.unlock ();
    }

    closeTriggerPipeOne ();
    closeTriggerPipeTwo ();
    closeEfd ();
}

}

