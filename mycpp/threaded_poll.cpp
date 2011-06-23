#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/threaded_poll.h>

namespace MyCpp {

void
ThreadedPoll::do_primary_poll ()
{
    try {
	for (;;) {
	    IoActor::Events events;
	    Ref<Pollable> pollable = poll_group->poll (&events);

	    if (events &&
		!pollable.isNull ())
	    {
		pollable->processEvents (events);
	    }

	    pollable = NULL;

	    state_mutex.lock ();

	    if (should_stop) {
		state_mutex.unlock ();
		return;
	    }

	    state_mutex.unlock ();
	}
    } catch (Exception &exc) {
	errf->print ("MyCpp.ThreadedPoll.do_primary_poll: WARNING: exception: ");
	printException (errf, exc);
	return;
    }

    abortIfReached ();
}

void
ThreadedPoll::worker_thread_proc (void *_data)
{
    WorkerThreadData * const &data = static_cast <WorkerThreadData*> (_data);

    try {
	Ref<ThreadedPoll> self;
	for (;;) {
	    if (self.isNull ()) {
		self = data->weak_threaded_poll.getRef ();
		if (self.isNull ())
		    return;
	    }

	    Ref<ActivePollGroup> tmp_poll_group = self->poll_group;
	    self = NULL;

	    IoActor::Events events;
	    Ref<Pollable> pollable = tmp_poll_group->poll (&events);

	    if (events &&
		!pollable.isNull ())
	    {
		pollable->processEvents (events);
	    }

	    pollable = NULL;

	    self = data->weak_threaded_poll.getRef ();
	    if (self.isNull ())
		return;

	    self->state_mutex.lock ();

	    if (self->should_stop) {
		self->state_mutex.unlock ();
		return;
	    }

	    self->state_mutex.unlock ();
	}
    } catch (Exception &exc) {
	errf->print ("MyStorage.ThreadedPoll.worker_thread_proc: exception: ");
	printException (errf, exc);
	return;
    }

    abortIfReached ();
}

// TODO: Simultaneous invocations of run() will lead to
// too much threads spawned.
void
ThreadedPoll::run ()
{
    state_mutex.lock ();

    if (should_stop) {
/* Deprecated behavior, a separate method for cancelling
 * the stop() operation should be introduced for this.
 *
	should_stop = false;
	stopped_cond.signal ();
 */
	state_mutex.unlock ();
	return;
    }

    abortIf (num_threads == 0);
    for (Size i = 0; i < num_threads - 1; i++) {
	Ref<WorkerThreadData> wt_data = grab (new WorkerThreadData);
	wt_data->weak_threaded_poll = this;
	Ref<Thread> thread = grab (new Thread (worker_thread_proc,
					       wt_data,
					       wt_data,
					       true /* joinable */));
	wt_data->threads_link = threads.append (thread);
    }

    state_mutex.unlock ();

    do_primary_poll ();
}

// TODO: Simultaneous invocations of spawn() will lead to
// too much threads spawned.
void
ThreadedPoll::spawn ()
{
    state_mutex.lock ();

    if (should_stop) {
/* Deprecated behavior, a separate method for cancelling
 * the stop() operation should be introduced for this.
 *
	should_stop = false;
	stopped_cond.signal ();
 */
	state_mutex.unlock ();
	return;
    }

    for (Size i = 0; i < num_threads; i++) {
	Ref<WorkerThreadData> wt_data = grab (new WorkerThreadData);
	wt_data->weak_threaded_poll = this;
	Ref<Thread> thread = grab (new Thread (worker_thread_proc,
					       wt_data,
					       wt_data,
					       true /* joinable */));
	wt_data->threads_link = threads.append (thread);
    }

    state_mutex.unlock ();
}

void
ThreadedPoll::stop ()
{
    state_mutex.lock ();
    should_stop = true;
    state_mutex.unlock ();

    poll_group->trigger ();
}

void
ThreadedPoll::join_internal (bool /* but_one */)
{
    state_mutex.lock ();

    if (!should_stop) {
	errf->print ("MyCpp.ThreadedPoll.join_internal: "
		     "WARNING: stop() has not been called")
	     .pendl ();
    }

    while (threads.first != NULL) {
	Ref<Thread> thread = threads.first->data;
	abortIf (thread.isNull ());
	threads.remove (threads.first);
	state_mutex.unlock ();

	// TODO: For simultaneous invocations of join_internal()
	// some join_internal() invocations may return with
	// a number of threads not actually joined. This is not
	// an issue right now, but the semantics is distrupted.
	//
	// 09.09.30 Note: See SimpleThreadedPoller for a better
	// way of joining.
	thread->join ();

	state_mutex.lock ();
    }

    state_mutex.unlock ();
}

void
ThreadedPoll::joinAll ()
{
    join_internal (false /* but_one */);
}

void
ThreadedPoll::joinAllButOne ()
{
    join_internal (true /* but_one */);
}

ThreadedPoll::ThreadedPoll (ActivePollGroup *poll_group,
			    Size num_threads)
{
    abortIf (poll_group == NULL);
    abortIf (num_threads == 0);

    this->poll_group = poll_group;
    this->num_threads = num_threads;
}

ThreadedPoll::~ThreadedPoll ()
{
    stop ();
}

}

