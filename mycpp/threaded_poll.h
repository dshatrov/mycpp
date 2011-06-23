#ifndef __MYCPP__THREADED_POLL_H__
#define __MYCPP__THREADED_POLL_H__

#include <mycpp/list.h>
#include <mycpp/data_mutex.h>
#include <mycpp/thread.h>
#include <mycpp/active_poll_group.h>

namespace MyCpp {

class ThreadedPoll : public virtual Object
{
protected:
    class WorkerThreadData : public SimplyReferenced
    {
    public:
      // mt_const

	WeakRef<ThreadedPoll> weak_threaded_poll;

      // (end mt_const)

      // mt_mutex ThreadedPoll::state_mutex

	List< Ref<Thread> >::Element *threads_link;

      // (end mt_mutex ThreadedPoll::state_mutex)
    };

  // mt_const

    Size num_threads;

    Ref<ActivePollGroup> poll_group;

  // (end mt_const)

  // mt_mutex state_mutex

    List< Ref<Thread> > threads;

    Bool should_stop;

  // (end mt_mutex state_mutex)

    DataMutex state_mutex;

    // Polling method for independent polling threads.
    static void worker_thread_proc (void *_sti);

    // Polling method for a thread which has called run().
    void do_primary_poll ();

    void join_internal (bool but_one);

public:
    // run() makes the current thread one of the polling threads
    // of the ThreadedPoll object, and spawns the rest of the polling threads.
    void run ();

    // spawn() spawns polling threads and returns control, i.e. it doesn't
    // make the current the current thread one of the polling threads.
    void spawn ();

    // TODO Compile all the code with this method commented out.
    // Its semantics has changed.
    //
    // Signals that all polling threads should be stopped, but doesn't
    // wait for the threads to stop and returns immediately.
    void stop ();

    // Waits for all of the polling threads to stop.
    //
    // One should call stop() prior to calling joinAll(),
    // a warning will be generated otherwise.
    void joinAll ();

    // Wait for all but one of the polling threads to stop.
    //
    // This method is useful when the caller knows that he is calling
    // joinAllButOne() from a polling thread. In this case it guarantees
    // that all the other polling threads have finished execution.
    //
    // One should call stop() prior to calling joinAllButOne(),
    // a warning will be generated otherwise.
    void joinAllButOne ();

    ThreadedPoll (ActivePollGroup *poll_group,
		  Size num_threads);

    ~ThreadedPoll ();
};

}

#endif /* __MYCPP__THREADED_POLL_H__ */

