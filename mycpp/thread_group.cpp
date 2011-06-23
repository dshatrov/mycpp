#include <mycpp/thread_group.h>

namespace MyCpp {

void
ThreadGroup::threadGroupProc (void *_st_info)
{
    // Both 'st_info' and 'self' are guaranteed to be valid,
    // because 'self' serves as a weak_obj for this callback,
    // and ScheduledTaskInfo can only be destroyed from this
    // callback or from ThreadGroup's destructor (which hasn't
    // been called at this point, because 'self' is valid).
    ScheduledTaskInfo * const &st_info = static_cast <ScheduledTaskInfo*> (_st_info);
    ThreadGroup *self = st_info->threadGroup;

    self->tasksMutex.lock ();

    if (self->nActiveThreads <= self->maxThreads) {
	Ref<TaskRecord> tr;

	while (self->pendingTasks.first != NULL) {
	    tr = self->pendingTasks.first->data;
	    self->pendingTasks.remove (self->pendingTasks.first);

	    tr->valid = false;
	    tr->tasksLink = NULL;

	    if (tr->cb.refCallback != NULL)
		tr->cb.refCallback (tr->cb.refData);

	    self->tasksMutex.unlock ();

	    if (tr->cb.callback != NULL)
		tr->cb.callback (tr->cb.callbackData);

	    if (tr->cb.unrefCallback != NULL)
		tr->cb.unrefCallback (tr->cb.refData);

	    self->tasksMutex.lock ();
	}
    }

    if (st_info->valid) {
	st_info->valid = false;
	self->scheduledTasks.remove (st_info->scheduledTasksLink);
	st_info->scheduledTasksLink = NULL;
    }

    self->nActiveThreads --;

    self->tasksMutex.unlock ();
}

Ref<Scheduler::TaskRecord>
ThreadGroup::scheduleTask (const CallbackDesc <TaskCallback> &cb)
{
    tasksMutex.lock ();

    Ref<TaskRecord> taskRecord = grab (new TaskRecord);
    taskRecord->valid = true;
    taskRecord->cb.setDesc (&cb);
    taskRecord->tasksLink = pendingTasks.append (taskRecord);

    if (nActiveThreads < maxThreads) {
	nActiveThreads ++;

	Ref<ScheduledTaskInfo> sti = grab (new ScheduledTaskInfo);
	sti->valid = true;
	sti->threadGroup = this;
	sti->scheduledTasksLink = scheduledTasks.append (sti);

	tasksMutex.unlock ();

	Ref<Scheduler::TaskRecord> sched_tr;
	{
	    CallbackDesc<TaskCallback> cb;
	    cb.weak_obj = this;
	    cb.callback = threadGroupProc;
	    cb.callbackData  = sti;

	    sched_tr = threadPool->scheduleTask (cb);
	}

	tasksMutex.lock ();

	if (sti->valid)
	    sti->sched_tr = sched_tr;
    }

    if (!taskRecord->valid)
	taskRecord = NULL;

    tasksMutex.unlock ();

//    return takeOver<Scheduler::TaskRecord> (taskRecord);
    return taskRecord.ptr ();
}

void
ThreadGroup::cancelTask (Scheduler::TaskRecord *_taskRecord)
{
    TaskRecord *taskRecord = static_cast <TaskRecord*> (_taskRecord);

    tasksMutex.lock ();
    if (taskRecord->valid) {
	taskRecord->valid = false;
	if (taskRecord->tasksLink != NULL) {
	    pendingTasks.remove (taskRecord->tasksLink);
	    taskRecord->tasksLink = NULL;
	}
    }
    tasksMutex.unlock ();
}

void
ThreadGroup::setMaxThreads (unsigned long maxThreads)
{
    tasksMutex.lock ();
    this->maxThreads = maxThreads;
    /* Could issue more tasks here... */
    tasksMutex.unlock ();
}

ThreadGroup::ThreadGroup (ThreadPool    *threadPool,
			  unsigned long  maxThreads)
{
    this->threadPool = threadPool;
    this->maxThreads = maxThreads;

    nActiveThreads = 0;
}

ThreadGroup::~ThreadGroup ()
{
    tasksMutex.lock ();

    {
	List< Ref<ScheduledTaskInfo> >::DataIterator sti_iter (scheduledTasks);
	while (!sti_iter.done ()) {
	    Ref<ScheduledTaskInfo> &sti = sti_iter.next ();
	    sti->valid = false;
	    sti->scheduledTasksLink = NULL;
	    if (!sti->sched_tr.isNull ()) {
		threadPool->cancelTask (sti->sched_tr);
		sti->sched_tr = NULL;
	    }
	}
	scheduledTasks.clear ();
    }

    {
	List< Ref<TaskRecord> >::DataIterator tr_iter (pendingTasks);
	while (!tr_iter.done ()) {
	    Ref<TaskRecord> &tr = tr_iter.next ();
	    tr->cb.reset ();
	    tr->valid = false;
	    tr->tasksLink = NULL;
	}
	pendingTasks.clear ();
    }

    tasksMutex.unlock ();
}

}

