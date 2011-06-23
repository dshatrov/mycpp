#ifndef __MYCPP__POLL_GROUP_H__
#define __MYCPP__POLL_GROUP_H__

#include <mycpp/object.h>
#include <mycpp/pollable.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

/*c A group of <t>Pollable</t> objects
 * which are to be polled simultaneously. */
class PollGroup : public virtual Object
{
public:
    class PollableRecord : public virtual SimplyReferenced
    {
    };

    class RecordHolder : public virtual SimplyReferenced
    {
    protected:
	WeakRef<PollGroup> weak_poll_group;
	Ref<PollableRecord> pollable_record;

    public:
	RecordHolder (PollGroup      *poll_group,
		      PollableRecord *pollable_record)
	{
	    this->weak_poll_group = poll_group;
	    this->pollable_record = pollable_record;
	}

	~RecordHolder ()
	{
	    Ref<PollGroup> poll_group = weak_poll_group.getRef ();

	    if (!poll_group.isNull () &&
		!pollable_record.isNull ())
	    {
		poll_group->removePollable (pollable_record);
	    }
	}
    };

public:
    /*m Adds a new <t>Pollable</t> object to the group.
     *
     * Adding a Pollable object to the group doesn't add a reference
     * to that Pollable. Pollables are automatically removed from the group
     * when the target object gets destroyed.
     *
     * Can return a null reference in cases when the Pollable is pre-closed.
     *
     * @pollable The <t>Pollable</t> object to be added to the group. */
    virtual Ref<PollableRecord> addPollable (Pollable *pollable,
					     IoActor::Events events =
						     IoActor::EventRead  |
						     IoActor::EventWrite |
						     IoActor::EventError)
				      throw (InternalException) = 0;

    /*m*/
    virtual void removePollable (PollableRecord *pr)
			  throw (InternalException) = 0;
};

}

#endif /* __MYCPP__POLL_GROUP_H__ */

