#ifndef __MYCPP__BUSY_POLL_GROUP_H__
#define __MYCPP__BUSY_POLL_GROUP_H__

#include <mycpp/object.h>
#include <mycpp/data_mutex.h>
#include <mycpp/active_poll_group.h>

namespace MyCpp {

class BusyPollGroup : public ActivePollGroup,
		      public virtual Object
{
public:
    class PollableRecord : public PollGroup::PollableRecord,
			   public virtual Object
    {
	friend class BusyPollGroup;

    protected:
      // mt_mutex BusyPollGroup::state_mutex

	bool valid;
	WeakRef<Pollable> weak_pollable;
	IoActor::Events events;
//	Ref<DeletionSubscription> del_sbn;
	DeletionSubscriptionKey del_sbn;
	Ref<GenericInformer::Subscription> need_input_sbn;
	Ref<GenericInformer::Subscription> need_output_sbn;
	List< Ref<PollableRecord> >::Element *pollables_link;

      // (end mt_mutex BusyPollGroup::state_mutex)
    };

protected:

  // mt_mutex state_mutex

    List< Ref<PollableRecord> > pollables;

  // (end mt_mutex state_mutex)

    DataMutex state_mutex;

    static void pollable_deletion_callback (void *_data);

    static void need_input_callback (bool  need,
				     void *_data);

    static void need_output_callback (bool  need,
				      void *_data);

public:
  // interface ActivePollGroup

    Ref<PollGroup::PollableRecord> addPollable (Pollable *pollable,
						IoActor::Events events)
					 throw (InternalException);

    void removePollable (PollGroup::PollableRecord *pr)
		  throw (InternalException);

    Ref<Pollable> poll (IoActor::Events *ret_events)
		 throw (InternalException);

    void trigger ()
	   throw (InternalException);

  // (end interface ActivePollGroup)
};

}

#endif /* __MYCPP__BUSY_POLL_GROUP_H__ */

