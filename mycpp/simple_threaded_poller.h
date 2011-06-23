#ifndef __MYCPP__SIMPLE_THREADED_POLLER_H__
#define __MYCPP__SIMPLE_THREADED_POLLER_H__

#include <mycpp/list.h>
#include <mycpp/data_mutex.h>
#include <mycpp/object.h>
#include <mycpp/thread.h>
#include <mycpp/poller.h>
#include <mycpp/active_poll_group.h>

namespace MyCpp {

class SimpleThreadedPoller : public Poller,
			     public virtual Object
{
protected:
    class ThreadRecord;

    class PollableRecord : public PollGroup::PollableRecord,
			   public virtual Object
    {
    public:
      // mt_const

	WeakRef<SimpleThreadedPoller> weak_self;
	WeakRef<Pollable> weak_pollable;

	Ref<GenericInformer::Subscription> pre_closed_sbn;
	DeletionSubscriptionKey del_sbn;

      // (end mt_const)

      // mt_mutex SimpleThreadedPoller::state_mutex

	bool valid;

	Ref<PollGroup::PollableRecord> group_record;

	WeakRef<ThreadRecord> weak_thread_record;
	List< Ref<PollableRecord> >::Element *pollables_link;

      // (end mt_mutex SimpleThreadedPoller::state_mutex)
    };

    class ThreadData : public virtual SimplyReferenced
    {
    public:
      // mt_const

	WeakRef<SimpleThreadedPoller> weak_self;
	Ref<ActivePollGroup> active_poll_group;

      // (end mt_const)

      // mt_mutex SimpleThreadedPoller::state_mutex

	Bool should_stop;
	List< Ref<PollableRecord> > pollables;

      // (end mt_mutex SimpleThreadedPoller::state_mutex)
    };

    class ThreadRecord : public virtual Object
    {
    public:
      // mt_const

	Ref<ThreadData> thread_data;
	Ref<Thread> thread;

      // (end mt_const)

      // mt_mutex SimpleThreadedPoller::state_mutex

	List< Ref<ThreadRecord> >::Element *threads_link;

	Bool joined;

      // (end mt_mutex SimpleThreadedPoller::state_mutex)
    };

  // mt_const

    Ref< ObjectFactory<ActivePollGroup> > factory;
    Size slots_per_thread;

  // (end mt_const)

  // mt_mutex state_mutex

    Bool run_in_progress,
	 run_complete;
    Ref<ThreadData> run_thread_data;

    Bool sorting_threads;

    List< Ref<ThreadRecord> > busy_threads;
    List< Ref<ThreadRecord> > spare_threads;

    Size total_free_slots;

    Bool should_stop;

  // (end mt_mutex state_mutex)

    DataMutex state_mutex;

    void releasePollable (PollableRecord *pr)
		   throw (InternalException);

    static void pollable_pre_closed_callback (void *_pr);

    static void pollable_deletion_callback (void *_pr);

    static void poll_thread_func (void *_thread_data);

public:
  // interface Poller

    Ref<PollGroup::PollableRecord> addPollable (Pollable *pollable,
						IoActor::Events events)
					 throw (InternalException);

    void removePollable (PollGroup::PollableRecord *pr)
		  throw (InternalException);

    void run ()
       throw (InternalException);

    void stopNoJoin ()
	      throw (InternalException);

    void join ()
	throw (InternalException);

  // (end interface Poller)

    /*m The constructor.
     * @factory - the mean of creating ActivePollGroup objects for the poller.
     * @slots_per_thread - maximum number of simultaneously polled objects per thread.
     */
    SimpleThreadedPoller (ObjectFactory<ActivePollGroup> *factory,
			  Size slots_per_thread);

    /*m The destructor.
     */
    ~SimpleThreadedPoller ()
		    throw (InternalException);
};

}

#endif /* __MYCPP__SIMPLE_THREADED_POLLER_H__ */

