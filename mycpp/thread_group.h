#ifndef __MYCPP__THREAD_GROUP_H__
#define __MYCPP__THREAD_GROUP_H__

#include <mycpp/list.h>
#include <mycpp/data_mutex.h>
#include <mycpp/scheduler.h>
#include <mycpp/thread_pool.h>

namespace MyCpp {

// NOTE 08.07.07: This class is unused to date, I only keep it around
// because the implementation looks sane. It's not tested though,
// and thus it's not guaranteed to be suitable for any purpose.

class ThreadGroup : public Scheduler,
		    public virtual Object
{
protected:
    struct ScheduledTaskInfo : public SimplyReferenced
    {
	bool valid;
	ThreadGroup *threadGroup;
	List< Ref<ScheduledTaskInfo> >::Element *scheduledTasksLink;

	Ref<Scheduler::TaskRecord> sched_tr;
    };

    struct TaskRecord : public Scheduler::TaskRecord
    {
	bool valid;
	List< Ref<TaskRecord> >::Element *tasksLink;

	CallbackDesc<TaskCallback> cb;
    };

    Ref<ThreadPool> threadPool;

    List< Ref<TaskRecord> > pendingTasks;
    List< Ref<ScheduledTaskInfo> > scheduledTasks;

    DataMutex tasksMutex;

    unsigned long nActiveThreads;
    unsigned long maxThreads;

    static void threadGroupProc (void *_st_info);

public:
  /* Scheduler interface */

    Ref<Scheduler::TaskRecord> scheduleTask (const CallbackDesc<TaskCallback> &cb);

    void cancelTask (Scheduler::TaskRecord *taskRecord);

  /* (End of Scheduler interface) */

    void setMaxThreads (unsigned long maxThreads);

    /* TODO ThreadPool can now be replaced by a Scheduler. */
    ThreadGroup (ThreadPool    *threadPool,
		 unsigned long  maxThreads);

    ~ThreadGroup ();
};

}

#endif /* __MYCPP__THREAD_GROUP_H__ */

