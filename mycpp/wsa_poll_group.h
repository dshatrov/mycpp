#ifndef __MYCPP__WSA_POLL_GROUP_H__
#define __MYCPP__WSA_POLL_GROUP_H__

#ifndef PLATFORM_WIN32
#error
#endif

#include <mycpp/object.h>
#include <mycpp/list.h>
#include <mycpp/data_mutex.h>
#include <mycpp/active_poll_group.h>
#include <mycpp/object_factory.h>

namespace MyCpp {

class WsaPollGroup : public ActivePollGroup,
		     public virtual Object
{
public:
    class PollableRecord : public PollGroup::PollableRecord,
			   public virtual Object
    {
	friend class WsaPollGroup;

    protected:
      // mt_const

	WeakRef<WsaPollGroup> weak_wsa_poll_group;
	WeakRef<Pollable> weak_pollable;

      // (end mt_const)

      // mt_mutex WsaPollGroup::state_mutex

	IoActor::Events events;

	bool valid;
	WSAEVENT wsa_event;

	Ref<GenericInformer::Subscription> input_sbn,
					   output_sbn,
					   pre_closed_sbn;
	Ref<DeletionSubscription> del_sbn;

	List< Ref<PollableRecord> >::Element *pollables_link;

      // (end mt_mutex WsaPollGroup::state_mutex)
    };

protected:
  // mt_mutex state_mutex

    WSAEVENT trigger_event;

    List< Ref<PollableRecord> > pollables;

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

    void removePollable (PollGroup::PollableRecord *pr)
		  throw (InternalException);

    Ref<Pollable> poll (IoActor::Events *events,
			int              timeout)
		 throw (InternalException);

    void trigger ()
	   throw (InternalException);

  // (end interface ActivePollGroup)

    WsaPollGroup ()
	   throw (InternalException);

    ~WsaPollGroup ()
	   throw (InternalException);

    static Size getMaxWaitObjects ();

    static Ref< ObjectFactory<ActivePollGroup> > getFactory ();
};

}

#endif /* __MYCPP__WSA_POLL_GROUP_H__ */

