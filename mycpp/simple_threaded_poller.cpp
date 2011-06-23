#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/simple_threaded_poller.h>

#define DEBUG(a) ;
#define DEBUG_POLL(a) ;

namespace MyCpp {

void
SimpleThreadedPoller::releasePollable (PollableRecord *pr)
    throw (InternalException)
{
    // TEST
//    return;

    abortIf (pr == NULL);
    state_mutex.assertLocked ();

    if (!pr->valid)
	return;
    pr->valid = false;

    {
	Ref<Pollable> pollable = pr->weak_pollable.getRef ();
	if (!pollable.isNull ()) {
	    pollable->getPreClosedInformer ()->unsubscribe (pr->pre_closed_sbn);
	    pollable->removeDeletionCallback (pr->del_sbn);
	}
    }

    if (!pr->weak_thread_record.isValid ()) {
	if (!run_thread_data.isNull ()) {
	    // FIXME throws
	    run_thread_data->active_poll_group->removePollable (pr->group_record);
	    if (pr->pollables_link != NULL) {
		run_thread_data->pollables.remove (pr->pollables_link);
		pr->pollables_link = NULL;
	    }
	}
    } else {
	Ref<ThreadRecord> thread_record = pr->weak_thread_record.getRef ();
	if (thread_record.isNull ())
	    return;

	// FIXME throws
	thread_record->thread_data->active_poll_group->removePollable (pr->group_record);

	if (pr->pollables_link != NULL) {
	    thread_record->thread_data->pollables.remove (pr->pollables_link);
	    pr->pollables_link = NULL;
	}

	if (thread_record->thread_data->pollables.getNumElements () == slots_per_thread - 1) {
	    busy_threads.remove (thread_record->threads_link);
	    thread_record->threads_link = spare_threads.append (thread_record);
	}

	// TODO if (...pollables.getNumElements () == 0) { ... }
    }

    total_free_slots ++;

    if (sorting_threads)
	return;
    sorting_threads = true;

    try {
	while (total_free_slots >= slots_per_thread &&
	       !spare_threads.isEmpty ())
	{
	    Ref<ThreadRecord> giving_thread;
	    bool giving_thread_busy;
	    if (!busy_threads.isEmpty ()) {
		giving_thread_busy = true;
		giving_thread = busy_threads.first->data;
	    } else {
		abortIf (spare_threads.isEmpty ());
		giving_thread_busy = false;
		giving_thread = spare_threads.last->data;
	    }

	    Ref<ThreadRecord> accepting_thread;
	    Ref<ThreadData> accepting_data;
	    if (!busy_threads.isEmpty () ||
		(spare_threads.last != spare_threads.first))
	    {
		accepting_thread = spare_threads.first->data;
		accepting_data = accepting_thread->thread_data;
	    } else {
		accepting_data = run_thread_data;
	    }
	    abortIf (static_cast <ThreadRecord*> (giving_thread) == static_cast <ThreadRecord*> (accepting_thread));

	    List< Ref<PollableRecord> >::Element *cur_pr_el = NULL;
	    List< Ref<PollableRecord> >::Element *next_pr_el = giving_thread->thread_data->pollables.first;;
	    for (;;) {
		cur_pr_el = next_pr_el;
		if (cur_pr_el == NULL)
		    break;

		next_pr_el = cur_pr_el->next;

		Ref<PollableRecord> pr = cur_pr_el->data;
		abortIf (static_cast <ThreadRecord*> (pr->weak_thread_record.getRef ()) != static_cast <ThreadRecord*> (giving_thread));
		abortIf (pr->pollables_link != cur_pr_el);
		giving_thread->thread_data->pollables.remove (cur_pr_el);
		// FIXME throws
		giving_thread->thread_data->active_poll_group->removePollable (pr->group_record);

		Ref<Pollable> pollable = pr->weak_pollable.getRef ();
		if (!pollable.isNull ()) {
		    // FIXME throws
		    pr->group_record = accepting_data->active_poll_group->addPollable (pollable, IoActor::AllEvents);
		    pr->weak_thread_record = accepting_thread;
		    pr->pollables_link = accepting_data->pollables.append (pr);

		    abortIf (accepting_data->pollables.getNumElements () > slots_per_thread);
		    if (accepting_data->pollables.getNumElements () == slots_per_thread) {
			if (!accepting_thread.isNull ()) {
			    spare_threads.remove (accepting_thread->threads_link);
			    accepting_thread->threads_link = busy_threads.append (accepting_thread);
			}

			if (!spare_threads.isEmpty ()) {
			    accepting_thread = spare_threads.first->data;
			    accepting_data = accepting_thread->thread_data;
			} else {
			    accepting_thread = NULL;
			    accepting_data = run_thread_data;
//			    abortIf (run_thread_data->pollables.getNumElements () == slots_per_thread &&
//				     total_free_slots > 0);
			}
		    }

		    if (total_free_slots == 0)
			break;
		} else {
		    total_free_slots ++;
		}
	    }

	    if (giving_thread->thread_data->pollables.isEmpty ()) {
		DEBUG (
		    errf->print ("MyCpp.SimpleThreadedPoller.releasePollable: "
				 "stopping thread").pendl ();
		)

		giving_thread->thread_data->should_stop = true;
		giving_thread->thread_data->active_poll_group->trigger ();
		if (giving_thread_busy) {
		    busy_threads.remove (giving_thread->threads_link);
		} else {
		    abortIf (spare_threads.isEmpty ());
		    spare_threads.remove (giving_thread->threads_link);
		}

		abortIf (total_free_slots < slots_per_thread);
		total_free_slots -= slots_per_thread;

// TEST
#if 0
		state_mutex.unlock ();

		try {
		    // FIXME Attempting to join current thread.
		    giving_thread->thread->join ();
		} catch (Exception &exc) {
		    state_mutex.lock ();
		    sorting_threads = false;
		    exc.raise ();
		}

		state_mutex.lock ();
#endif
	    } else {
		if (giving_thread_busy &&
		    giving_thread->thread_data->pollables.getNumElements () < slots_per_thread)
		{
		    busy_threads.remove (giving_thread->threads_link);
		    giving_thread->threads_link = spare_threads.append (giving_thread);
		}
	    }
	}
    } catch (Exception &exc) {
	sorting_threads = false;
	exc.raise ();
    }

    sorting_threads = false;
}

void
SimpleThreadedPoller::pollable_pre_closed_callback (void *_pr)
{
    DEBUG (
	errf->print ("MyCpp.SimpleThreadedPoller.pollable_pre_closed_callback").pendl ();
    )

    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);
    Ref<SimpleThreadedPoller> self = pr->weak_self.getRef ();
    if (self.isNull ())
	return;

    try {
	self->removePollable (pr);
    } catch (Exception &exc) {
	errf->print ("MyCpp.SimpleThreadedPoller.pollable_pre_closed_callback: ");
	printException (errf, exc);
    }
}

void
SimpleThreadedPoller::pollable_deletion_callback (void *_pr)
{
    DEBUG (
	errf->print ("MyCpp.SimpleThreadedPoller.pollable_deletion_callback").pendl ();
    )

    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);
    Ref<SimpleThreadedPoller> self = pr->weak_self.getRef ();
    if (self.isNull ())
	return;

    try {
	self->removePollable (pr);
    } catch (Exception &exc) {
	errf->print ("MyCpp.SimpleThreadedPoller.pollable_deletion_callback: ");
	printException (errf, exc);
    }
}

void
SimpleThreadedPoller::poll_thread_func (void *_thread_data)
try {
    DEBUG (
	errf->print ("MyCpp.SimpleThreadedPoller.poll_thread_func").pendl ();
    )

    ThreadData * const &thread_data = static_cast <ThreadData*> (_thread_data);

    for (;;) {
//	DEBUG (
//	    errf->print ("MyCpp.SimpleThreadedPoller.poll_thread_func: iteration").pendl ();
//	)
	{
	    Ref<SimpleThreadedPoller> self = thread_data->weak_self.getRef ();
	    if (self.isNull ())
		break;

	  DataMutexLock state_lock (self->state_mutex);

	    if (self->should_stop)
		break;

	    if (thread_data->should_stop)
		break;
	}

//	DEBUG (
//	    errf->print ("MyCpp.SimpleThreadedPoller.poll_thread_func: calling iteration()").pendl ();
//	)
	thread_data->active_poll_group->iteration ();
    }

    DEBUG (
	errf->print ("MyCpp.SimpleThreadedPoller.poll_thread_func: done").pendl ();
    )
} catch (Exception &exc) {
    errf->print ("MyCpp.SimpleThreadedPoller.poll_thread_func: ");
    printException (errf, exc);
}

Ref<PollGroup::PollableRecord>
SimpleThreadedPoller::addPollable (Pollable *pollable,
				   IoActor::Events events)
    throw (InternalException)
{
    abortIf (pollable == NULL);

    DEBUG_POLL (
	errf->print ("MyCpp.SimpleThreadedPoller.addPollable: ").print (pollable).pendl ();
    )

    Ref<PollableRecord> pr = grab (new PollableRecord);
    pr->valid = true;
    pr->weak_self = this;
    pr->weak_pollable = pollable;
    pr->pollables_link = NULL;

  DataMutexLock state_lock (state_mutex);

    try {
	if (run_complete) {
	    releasePollable (pr);
	    return NULL;
	}

	{
	    CallbackDesc<IoActor_Closing::PreClosedCallback> cb;
	    cb.weak_obj = this;
	    cb.callback = pollable_pre_closed_callback;
	    cb.callbackData = pr;
	    cb.addRefData (pr);

	    if (pollable->isPreClosed_subscribe (&cb, GenericInformer::InformOneshot, &pr->pre_closed_sbn)) {
		releasePollable (pr);
		return NULL;
	    }
	}

	{
#if 0
	    CallbackDesc<DeletionCallback> cb;
	    cb.weak_obj = this;
	    cb.callback = pollable_deletion_callback;
	    cb.callbackData = pr;
	    cb.addRefData (pr);

	    pr->del_sbn = pollable->addDeletionCallback (cb);
#endif
	    pr->del_sbn = pollable->addDeletionCallback (pollable_deletion_callback, pr, pr, this);
	}

	Ref<ThreadData> thread_data;
	if (run_thread_data->pollables.getNumElements () < slots_per_thread)
	    thread_data = run_thread_data;
	else
	if (!spare_threads.isEmpty ()) {
	    Ref<ThreadRecord> thread_record = spare_threads.first->data;
	    thread_data = thread_record->thread_data;
	    abortIf (thread_data->pollables.getNumElements () >= slots_per_thread);
	    if (thread_data->pollables.getNumElements () == slots_per_thread - 1) {
		spare_threads.remove (thread_record->threads_link);
		thread_record->threads_link = busy_threads.append (thread_record);
	    }
	    pr->weak_thread_record = thread_record;
	} else {
	    Ref<ThreadRecord> thread_record = grab (new ThreadRecord);
	    thread_record->thread_data = grab (new ThreadData);
	    thread_record->thread_data->weak_self = this;
	    thread_record->thread_data->active_poll_group = factory->createNew ();
	    thread_record->thread = grab (new Thread (poll_thread_func,
						      thread_record->thread_data,
						      thread_record->thread_data,
						      // TEST
						      false));
//						      true /* joinable */));
	    if (slots_per_thread > 1)
		thread_record->threads_link = spare_threads.append (thread_record);
	    else
		thread_record->threads_link = busy_threads.append (thread_record);
	    thread_data = thread_record->thread_data;
	    pr->weak_thread_record = thread_record;

	    total_free_slots += slots_per_thread;
	}

	// FIXME throws
	pr->group_record = thread_data->active_poll_group->addPollable (pollable, events);
	pr->pollables_link = thread_data->pollables.append (pr);

	total_free_slots --;

	return pr.ptr ();
    } catch (Exception &exc) {
	releasePollable (pr);
	exc.raise ();
    }

    abortIfReached ();
    return NULL;
}

void
SimpleThreadedPoller::removePollable (PollGroup::PollableRecord *_pr)
    throw (InternalException)
{
    if (_pr == NULL)
	return;

    PollableRecord * const &pr = static_cast <PollableRecord*> (_pr);

    Ref<SimpleThreadedPoller> self = pr->weak_self.getRef ();
    if (self.isNull ())
	return;

  DataMutexLock state_lock (self->state_mutex);

    releasePollable (pr);
}

void
SimpleThreadedPoller::run ()
    throw (InternalException)
{
    errf->print ("MyCpp.SimpleThreadedPoller.run").pendl ();

  DataMutexLock state_lock (state_mutex);

    if (run_in_progress)
	throw InternalException ();
    run_in_progress = true;

    for (;;) {
	state_lock.unlock ();
	run_thread_data->active_poll_group->iteration ();
	state_lock.lock ();
	if (should_stop)
	    break;
    }

    state_lock.unlock ();

    join ();

    state_lock.lock ();
    run_thread_data = NULL;
    run_complete = true;

    errf->print ("MyCpp.SimpleThreadedPoller.run: done").pendl ();
}

void
SimpleThreadedPoller::stopNoJoin ()
    throw (InternalException)
{
    errf->print ("MyCpp.SimpleThreadedPoller.stopNoJoin").pendl ();

  DataMutexLock state_lock (state_mutex);

    should_stop = true;

    {
	List< Ref<ThreadRecord> >::DataIterator threads_iter (busy_threads);
	while (!threads_iter.done ()) {
	    Ref<ThreadRecord> &tr = threads_iter.next ();
	    tr->thread_data->active_poll_group->trigger ();
	}
    }

    {
	List< Ref<ThreadRecord> >::DataIterator threads_iter (spare_threads);
	while (!threads_iter.done ()) {
	    Ref<ThreadRecord> &tr = threads_iter.next ();
	    tr->thread_data->active_poll_group->trigger ();
	}
    }
}

void
SimpleThreadedPoller::join ()
    throw (InternalException)
{
    errf->print ("MyCpp.SimpleThreadedPoller.join").pendl ();

  DataMutexLock state_lock (state_mutex);

    if (!should_stop) {
	state_lock.unlock ();
	stopNoJoin ();
	state_lock.lock ();
    }

    while (!busy_threads.isEmpty () && !spare_threads.isEmpty ()) {
	Ref<ThreadRecord> tr;
	if (!busy_threads.isEmpty ())
	    tr = busy_threads.first->data;
	else
	if (!spare_threads.isEmpty ())
	    tr = spare_threads.first->data;
	else
	    abortIfReached ();

	tr->joined = true;

// TEST
#if 0
	state_lock.unlock ();
	tr->thread->join ();
	state_lock.lock ();
#endif

	if (!busy_threads.isEmpty ())
	    tr = busy_threads.first->data;
	else
	if (!spare_threads.isEmpty ())
	    tr = spare_threads.first->data;
	else
	    break;

	if (tr->joined) {
	    if (!busy_threads.isEmpty ())
		busy_threads.remove (busy_threads.first);
	    else
	    if (!spare_threads.isEmpty ())
		spare_threads.remove (spare_threads.first);
	    else
		abortIfReached ();
	}
    }
}

SimpleThreadedPoller::SimpleThreadedPoller (ObjectFactory<ActivePollGroup> *factory,
					    Size slots_per_thread)
{
    abortIf (factory == NULL);
    this->factory = factory;

    this->slots_per_thread = slots_per_thread;
    total_free_slots = slots_per_thread;

    run_thread_data = grab (new ThreadData);
    run_thread_data->active_poll_group = factory->createNew ();
}

SimpleThreadedPoller::~SimpleThreadedPoller ()
    throw (InternalException)
{
    // Questionable: will deadlock if the destructor is called
    // in a polling thread.
    join ();
}

}

