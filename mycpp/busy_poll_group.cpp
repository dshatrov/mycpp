#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/busy_poll_group.h>

#define DEBUG(a) ;

using namespace MyCpp;

namespace MyCpp {

namespace BusyPollGroup_priv {}
using namespace BusyPollGroup_priv;

namespace BusyPollGroup_priv
{

    class DeletionData : public SimplyReferenced
    {
    public:
	WeakRef<BusyPollGroup> weak_self;
	WeakRef<BusyPollGroup::PollableRecord> weak_pr;
    };

}

void
BusyPollGroup::pollable_deletion_callback (void *_data)
{
    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.pollable_deletion_callback").pendl ();
    )

    DeletionData * const &data = static_cast <DeletionData*> (_data);
    Ref<BusyPollGroup> self = data->weak_self.getRef ();
    if (self.isNull ())
	return;
    Ref<PollableRecord> pr = data->weak_pr.getRef ();
    if (pr.isNull ())
	return;

  DataMutexLock state_lock (self->state_mutex);

    if (!pr->valid)
	return;
    pr->valid = false;

    self->pollables.remove (pr->pollables_link);
}

namespace BusyPollGroup_priv {

    class NeedInputData : public SimplyReferenced
    {
    public:
	WeakRef<BusyPollGroup> weak_self;
	WeakRef<BusyPollGroup::PollableRecord> weak_pr;
    };

}

void
BusyPollGroup::need_input_callback (bool need,
				    void *_data)
{
    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.need_input_callback").pendl ();
    )

    if (!need)
	return;

    NeedInputData * const &data = static_cast <NeedInputData*> (_data);
    Ref<BusyPollGroup> self = data->weak_self.getRef ();
    if (self.isNull ())
	return;
    Ref<PollableRecord> pr = data->weak_pr.getRef ();
    if (pr.isNull ())
	return;

  DataMutexLock state_lock (self->state_mutex);

    if (!pr->valid)
	return;

    pr->events |= IoActor::EventRead;
}

namespace BusyPollGroup_priv {

    class NeedOutputData : public SimplyReferenced
    {
    public:
	WeakRef<BusyPollGroup> weak_self;
	WeakRef<BusyPollGroup::PollableRecord> weak_pr;
    };

}

void
BusyPollGroup::need_output_callback (bool need,
				     void *_data)
{
    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.need_output_callback").pendl ();
    )

    if (!need)
	return;

    NeedOutputData * const &data = static_cast <NeedOutputData*> (_data);
    Ref<BusyPollGroup> self = data->weak_self.getRef ();
    if (self.isNull ())
	return;
    Ref<PollableRecord> pr = data->weak_pr.getRef ();
    if (pr.isNull ())
	return;

  DataMutexLock state_lock (self->state_mutex);

    if (!pr->valid)
	return;

    pr->events |= IoActor::EventWrite;
}

Ref<PollGroup::PollableRecord>
BusyPollGroup::addPollable (Pollable *pollable,
			    IoActor::Events events)
    throw  (InternalException)
{
    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.add_pollable").pendl ();
    )

    abortIf (pollable == NULL);

    Ref<PollableRecord> pr = grab (new PollableRecord);
    pr->valid = true;
    pr->weak_pollable = pollable;
    pr->events = events;

  DataMutexLock state_lock (state_mutex);

    {
	Ref<DeletionData> deletion_data = grab (new DeletionData);
	deletion_data->weak_self = this;
	deletion_data->weak_pr = pr;

	pr->del_sbn = pollable->addDeletionCallback (
                              M::CbDesc<Object::DeletionCallback> (
                                      pollable_deletion_callback,
                                      deletion_data,
                                      this,
                                      deletion_data));
    }

    {
	Ref<NeedInputData> need_input_data = grab (new NeedInputData);
	need_input_data->weak_self = this;
	need_input_data->weak_pr = pr;

	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = this;
	cb.callback = need_input_callback;
	cb.callbackData = need_input_data;
	cb.addRefData (need_input_data);

	pr->need_input_sbn = pollable->getNeedInputInformer ()->subscribe (cb);
    }

    {
	Ref<NeedOutputData> need_output_data = grab (new NeedOutputData);
	need_output_data->weak_self = this;
	need_output_data->weak_pr = pr;

	CallbackDesc<IoActor::NeedCallback> cb;
	cb.weak_obj = this;
	cb.callback = need_output_callback;
	cb.callbackData = need_output_data;
	cb.addRefData (need_output_data);

	pr->need_output_sbn = pollable->getNeedOutputInformer ()->subscribe (cb);
    }

    pr->pollables_link = pollables.append (pr);

    return pr.ptr ();
}

void
BusyPollGroup::removePollable (PollGroup::PollableRecord *_pr)
    throw (InternalException)
{
    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.remove_pollable").pendl ();
    )

    if (_pr == NULL)
	return;

    PollableRecord * const &pr = static_cast<PollableRecord*> (_pr);

  DataMutexLock state_lock (state_mutex);

    if (!pr->valid)
	return;
    pr->valid = false;

    {
	Ref<Pollable> pollable = pr->weak_pollable.getRef ();
	if (!pollable.isNull ()) {
	    pollable->removeDeletionCallback (pr->del_sbn);
	    pollable->getNeedInputInformer ()->unsubscribe (pr->need_input_sbn);
	    pollable->getNeedOutputInformer ()->unsubscribe (pr->need_output_sbn);
	}
    }
    
    pollables.remove (pr->pollables_link);
}

Ref<Pollable>
BusyPollGroup::poll (IoActor::Events *ret_events,
                     int              timeout_msec)
    throw (InternalException)
{
    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.poll").pendl ();
    )

    // TEST
    uSleep (100000);

    if (ret_events != NULL)
	*ret_events = 0;

  DataMutexLock state_lock (state_mutex);

    if (pollables.isEmpty ())
	return NULL;

    Ref<PollableRecord> pr = pollables.first->data;
    pollables.remove (pollables.first);
    pr->pollables_link = pollables.append (pr);

    IoActor::Events events = pr->events;
    pr->events = 0;

    Ref<Pollable> pollable = pr->weak_pollable.getRef ();
    if (pollable.isNull ())
	return NULL;

  state_lock.unlock ();

    if (events & IoActor::EventRead)
	pollable->processInput ();

    if (events & IoActor::EventWrite)
	pollable->processOutput ();

    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.poll: done").pendl ();
    )

    return NULL;
}

void
BusyPollGroup::trigger ()
    throw (InternalException)
{
    DEBUG (
	errf->print ("MyCpp.BusyPollGroup.trigger").pendl ();
    )

    // no-op
}

}

