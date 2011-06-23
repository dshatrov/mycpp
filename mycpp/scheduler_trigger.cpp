#include <mycpp/scheduler_trigger.h>
#include <mycpp/util.h>

namespace MyCpp {

void
SchedulerTrigger::schedulerTask (void *_self)
{
    SchedulerTrigger *self = static_cast <SchedulerTrigger*> (_self);

    self->stateMutex.lock ();

    while (self->needTrigger) {
	self->needTrigger = false;
	self->stateMutex.unlock ();

	self->fireEvent ();

	self->stateMutex.lock ();
    }

    self->task_scheduled = false;

    Ref<Scheduler::TaskRecord> tmp_tr = self->taskRecord;
    self->taskRecord = NULL;

    self->stateMutex.unlock ();

    tmp_tr = NULL;
}

void
SchedulerTrigger::trigger ()
{

    stateMutex.lock ();

    needTrigger = true;
    if (!task_scheduled) {
	task_scheduled = true;

	stateMutex.unlock ();

	CallbackDesc<Scheduler::TaskCallback> cb;
	cb.weak_obj = this;
	cb.callback = schedulerTask;
	cb.callbackData = this;

	Ref<Scheduler::TaskRecord> tmp_tr = scheduler->scheduleTask (cb);

	stateMutex.lock ();
	if (task_scheduled)
	    taskRecord = tmp_tr;
	stateMutex.unlock ();

	return;
    }

    stateMutex.unlock ();
}

SchedulerTrigger::SchedulerTrigger (Scheduler *scheduler)
{
    if (scheduler == NULL)
	abortIfReached ();

    this->scheduler = scheduler;
}

SchedulerTrigger::~SchedulerTrigger ()
{
    if (!taskRecord.isNull ()) {
	if (!task_scheduled)
	    abortIfReached ();

	scheduler->cancelTask (taskRecord);
    }
}

}

