#ifndef __MYCPP__SELECT_POLL_GROUP_H__
#define __MYCPP__SELECT_POLL_GROUP_H__

#include <mycpp/active_poll_group.h>
#include <mycpp/mutex.h>
#include <mycpp/data_mutex.h>
#include <mycpp/list.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

// TODO Update for the new Pollable semantics (PreClosedCallback)

/*c A group of <t>Pollable</t> objects
 * which are to be polled simultaneously. */
class SelectPollGroup : public ActivePollGroup,
			public virtual Object
{
public:
    class PollableRecord : public PollGroup::PollableRecord,
			   public virtual Object
    {
	friend class SelectPollGroup;

    protected:
	Ref<Pollable> pollable;
	WeakRef<SelectPollGroup> weak_group;

	Ref< Informer<IoActor::NeedCallback>::Subscription > needInputSbn,
							     needOutputSbn;

	bool needInput,
	     needOutput;

	IoActor::Events pendingEvents;

	bool valid;
	List< Ref<PollableRecord> >::Element *pollablesLink;
	List< Ref<PollableRecord> >::Element *activePollablesLink;
    };

protected:
    List< Ref<PollableRecord> > pollables;
    List< Ref<PollableRecord> > activePollables;

    DataMutex pollablesMutex;
    Mutex selectMutex;

    int triggerPipe [2];

    static void needInputEvent (bool  need,
				void *_pr);

    static void needOutputEvent (bool  need,
				 void *_pr);

    /* Must be called with pollablesMutex held. */
    Ref<PollableRecord> dequeueActivePollable (IoActor::Events *events);

    /* Must be called with pollablesMutex held. */
    void invalidatePollable (PollableRecord *pr);

    /* Call waitLock() to ensure that there is no thread
     * blocked in epoll_wait. */
    void waitLock ();
    void waitUnlock ();

public:
    Ref<PollGroup::PollableRecord> addPollable (Pollable *pollable,
						IoActor::Events events =
							IoActor::EventRead  |
							IoActor::EventWrite |
							IoActor::EventError)
					 throw (InternalException);

    void removePollable (PollGroup::PollableRecord *pr)
		  throw (InternalException);

    Ref<Pollable> poll (IoActor::Events *events,
			int              timeout)
		 throw (InternalException);

    void trigger ()
	   throw (InternalException);

    /*m The default constructor. */
    SelectPollGroup ()
	      throw (InternalException);

    ~SelectPollGroup ();
};

}

#endif /* __MYCPP__SELECT_POLL_GROUP_H__ */

