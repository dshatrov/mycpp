#ifndef __MYCPP__EPOLL_POLL_GROUP_H__
#define __MYCPP__EPOLL_POLL_GROUP_H__

#include <mycpp/active_poll_group.h>
#include <mycpp/data_mutex.h>
#include <mycpp/rw_lock.h>
#include <mycpp/list.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

/*c A group of <t>Pollable</t> objects
 * which are to be polled simultaneously. */
class EpollPollGroup : public ActivePollGroup,
		       public virtual Object
{
public:
    class PollableRecord : public PollGroup::PollableRecord,
			   public virtual Object
    {
	friend class EpollPollGroup;

    protected:
      // mt_const

	WeakRef<Pollable> weak_pollable;
	WeakRef<EpollPollGroup> weak_group;

     // (end mt_const)

     // mt_mutex EpollPollGroup::state_mutex

	Ref< Informer<IoActor::NeedCallback>::Subscription > pollable_pre_closed_sbn,
							     needInputSbn,
							     needOutputSbn;
	DeletionSubscriptionKey pollable_deletion_sbn;

	bool needInput,
	     needOutput;

	bool valid;
	List< Ref<PollableRecord> >::Element *groupLink;

      // (end mt_mutex EpollPollGroup::state_mutex)
    };

protected:
  // mt_const

    int efd;
    int triggerPipe [2];

  // (end mt_const)

  // mt_mutex state_mutex

    List< Ref<PollableRecord> > pollables;

  // (end mt_mutex state_mutex)

    DataMutex state_mutex;

    Mutex wait_mutex;
    RwLock waitRwLock;

    static void needInputEvent (bool  need,
				void *_pr);

    static void needOutputEvent (bool  need,
				 void *_pr);

    static void pollable_pre_closed_callback (void *_pr);

    static void pollable_deletion_callback (void *_pr);

    // Must be called with state_mutex held.
    void invalidatePollable (PollableRecord *pr);

    // Must be called with state_mutex held.
    void updatePollableEventMask (PollableRecord *pr);

    // Call waitLock() to ensure that there is no any threads
    // blocked in epoll_wait.
    void waitLock ();
    void waitUnlock ();

    // Helper wrappers around POSIX close(2) function
    void closeTriggerPipeOne ();
    void closeTriggerPipeTwo ();
    void closeEfd ();

public:
    Ref<PollGroup::PollableRecord> addPollable (Pollable *pollable,
						IoActor::Events events =
							IoActor::EventRead  |
							IoActor::EventWrite |
							IoActor::EventError)
					 throw (InternalException);

    void removePollable (PollGroup::PollableRecord *pr)
		  throw (InternalException);

    Ref<Pollable> poll (IoActor::Events * const events,
			int               const timeout)
		 throw (InternalException);

    void trigger ()
	   throw (InternalException);

    /*m The default constructor. */
    EpollPollGroup ()
	     throw (InternalException);

    ~EpollPollGroup ();
};

}

#endif /* __MYCPP__EPOLL_POLL_GROUP_H__ */

