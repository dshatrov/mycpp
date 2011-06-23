#ifndef __MYCPP__ACTIVE_POLL_GROUP_H__
#define __MYCPP__ACTIVE_POLL_GROUP_H__

#include <mycpp/object.h>
#include <mycpp/poll_group.h>
#include <mycpp/internal_exception.h>
#include <mycpp/object_factory.h>

namespace MyCpp {

class ActivePollGroup : public PollGroup,
			public virtual Object
{
public:
    /*m Waits until an event gets triggered.
     *
     * Returns the <t>Pollable</t> for which the event has been triggered.
     * Without any pollables in the group, poll() should block.
     *
     * @ret_events   A placeholder for the types of the triggered events, or NULL;
     * @timeout_msec Timeout in milliseconds. 0 means no timeout, -1 means infinite timeout. */
    virtual Ref<Pollable> poll (IoActor::Events *ret_events,
				int              timeout_msec = -1)
			 throw (InternalException) = 0;

    /*m Causes one of the threads blocked in a call to poll() to wake up. */
    virtual void trigger ()
		   throw (InternalException)= 0;

    /*m Typical polling iteration */
    void iteration ()
	     throw (InternalException)
    {
	IoActor::Events events;
	Ref<Pollable> pollable;

	pollable = poll (&events);

	if (!pollable.isNull ())
	    pollable->processEvents (events);
    }

    static Ref<ActivePollGroup> createDefault ()
					throw (InternalException);
};

Ref< ObjectFactory<ActivePollGroup> > getDefaultFactory_ActivePollGroup ();

}

#endif /* __MYCPP__ACTIVE_POLL_GROUP_H__ */

