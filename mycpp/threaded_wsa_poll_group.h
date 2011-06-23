#ifndef __MYCPP__THREADED_WSA_POLL_GROUP_H__
#define __MYCPP__THREADED_WSA_POLL_GROUP_H__

#ifndef PLATFORM_WIN32
#error
#endif

#include <mycpp/object.h>
#include <mycpp/list.h>
#include <mycpp/data_mutex.h>
#include <mycpp/active_poll_group.h>

namespace MyCpp {

class ThreadedWsaPollGroup : public ActivePollGroup,
			     public virtual Object
{
public:
    class PollableRecord : public PollGroup::PollableRecord,
			   public virtual Object
    {
	friend class ThreadedWsaPollGroup;

    protected:
      // mt_const

	WeakRef<ThreadedWsaPollGroup> weak_threaded_wsa_poll_group;
	WeakRef<Pollable> weak_pollable;

      // (end mt_const)

      // mt_mutex ThreadedWsaPollGroup::state_mutex

	IoActor::Events events;

	bool valid;
	WSAEVENT wsa_event;

	Ref<GenericInformer::Subscription> input_sbn,
					   output_sbn,
					   pre_closed_sbn;
	Ref<DeletionSubscription> del_sbn;

	List< Ref<PollableRecord> >::Element *pollables_link;

      // (end mt_mutex ThreadedWsaPollGroup::state_mutex)
    };

protected:
    class ThreadData : public virtual SimplyReferenced
    {
    public:
      // mt_mutex state_mutex

	List< Ref<Pollable> > pollables;

      // (end mt_mutex state_mutex)

	DataMutex state_mutex;
    };

  // mt_const

    Size slots_per_thread;

  // (end mt_const)

  // mt_mutex state_mutex

    WSAEVENT trigger_event;

    List< Ref<PollableRecord> > pollables;

    List< Ref<Thread> > busy_threads;

    List< Ref<Thread> > free_threads;

    Size total_free_slots;

    Size num_pollables;

  // (end mt_mutex state_mutex)

    DataMutex state_mutex;

    Mutex wait_mutex;

    static void need_input_callback (bool  need,
				     void *_pr);

    static void need_output_callback (bool  need,
				      void *_pr);

    static void pollable_pre_closed_callback (void *_pr);

    static void pollable_deletion_callback (void *_pr);

    void releasePollableRecord (PollableRecord *pr)
			 throw (InternalException);

public:
  // interface ActivePollGroup

    Ref<PollGroup::PollableRecord> addPollable (Pollable *pollable,
						IoActor::Events events)
					 throw (InternalException);

    void removePollable (PollGroup::PollableRecord *pr);

    Ref<Pollable> poll (IoActor::Events *events)
		 throw (InternalException);

    void trigger ();

  // (end interface ActivePollGroup)

    ThreadedWsaPollGroup ()
	   throw (InternalException);

    ~ThreadedWsaPollGroup ()
	   throw (InternalException);
};

}

#endif /* __MYCPP__THREADED_WSA_POLL_GROUP_H__ */

