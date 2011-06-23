#include <mycpp/poll_group_scheduler.h>

namespace MyCpp {

Ref<Scheduler::TaskRecord>
PollGroupScheduler::scheduleTask (const CallbackDesc<TaskCallback> &cbDesc)
{
    return NULL;
}

void
PollGroupScheduler::cancelTask (Scheduler::TaskRecord *taskRecord)
{
}

PollGroupScheduler::PollGroupScheduler (PollGroup *pollGroup)
{
    this->pollGroup = pollGroup;
}

}

