#ifndef __MYCPP_POLL_GROUP_SCHEDULER_H__
#define __MYCPP_POLL_GROUP_SCHEDULER_H__

#include <mycpp/scheduler.h>
#include <mycpp/poll_group.h>

namespace MyCpp {

class PollGroupScheduler : public Scheduler,
			   public virtual Object
{
protected:
    Ref<PollGroup> pollGroup;

public:
    Ref<Scheduler::TaskRecord>
	    scheduleTask (const CallbackDesc<TaskCallback> &cbDesc);

    void cancelTask (Scheduler::TaskRecord *taskRecord);

    PollGroupScheduler (PollGroup *pollGroup);
};

}

#endif /* __MYCPP_POLL_GROUP_SCHEDULER_H__ */

