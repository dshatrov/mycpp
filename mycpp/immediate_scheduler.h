#ifndef __MYCPP__IMMEDIATE_SCHEDULER_H__
#define __MYCPP__IMMEDIATE_SCHEDULER_H__

#include <mycpp/scheduler.h>
#include <mycpp/list.h>
#include <mycpp/immediate_trigger.h>

namespace MyCpp {

class ImmediateScheduler : public Scheduler,
			   public virtual Object
{
protected:
    class TaskRecord : public Scheduler::TaskRecord
    {
    public:
	bool valid;

	CallbackDesc<TaskCallback> cb;
	DeletionSubscriptionKey deletion_sbn;

	List< Ref<TaskRecord> >::Element *scheduler_link;

	ImmediateScheduler *immediate_scheduler;
    };

    Mutex tasks_mutex;

    ImmediateTrigger trigger;
    List< Ref<TaskRecord> > task_queue;

    static void triggerEventCallback (void *_self);

    static void clientDeletionCallback (void *_tr);

public:
    Ref<Scheduler::TaskRecord> scheduleTask (const CallbackDesc<TaskCallback> &cb);

    void cancelTask (Scheduler::TaskRecord *_taskRecord);

    ImmediateScheduler ();

    ~ImmediateScheduler ();
};

}

#endif /* __MYCPP__IMMEDIATE_SCHEDULER_H__ */

