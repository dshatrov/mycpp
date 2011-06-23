#ifndef __MYCPP__THREAD_POOL_H__
#define __MYCPP__THREAD_POOL_H__

#include <mycpp/scheduler.h>
#include <mycpp/thread.h>
#include <mycpp/list.h>
#include <mycpp/mutex.h>
#include <mycpp/data_mutex.h>
#include <mycpp/cond.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

class ThreadPool : public Scheduler,
		   public virtual Object
{
protected:
    class TaskRecord : public Scheduler::TaskRecord
    {
    public:
	bool valid;
	CallbackDesc<TaskCallback> cb;
	List< Ref<TaskRecord> >::Element *tasksLink;
    };

    class WorkerThread : public virtual SimplyReferenced
    {
    public:
	typedef void (*WorkerProc) (void *data);

    protected:
	Ref<Thread> thread;

      // Protected by activeMutex
	Bool  shouldStop;
	Bool  active;
	Cond  activeCond;
	Mutex activeMutex;
     // (End of what is protected by activeMutex)

	/* 'data' is of type "WorkerThread*" */
	static void workerThreadProc (void *_self);

    public:
	ThreadPool *thread_pool;

	List< Ref<WorkerThread> >::Element *storedLink;
	Ref<TaskRecord> storedTask;

	void resume ();

	void stop ();

	void join ();

	WorkerThread (ThreadPool *thread_pool)
	       throw (InternalException);
    };

    List< Ref<WorkerThread> > spareThreads;
    List< Ref<WorkerThread> > busyThreads;

    List< Ref<TaskRecord> > pendingTasks;

    unsigned long maxThreads,
		  minThreads;

    DataMutex state_mutex;

    // Must be called with 'state_mutex' held.
    void spawnSpareThreads () throw (InternalException);

    // Must be called with 'state_mutex' held.
    void activateWorkerThread (WorkerThread *workerThread,
			       TaskRecord   *taskRecord);

    static void workerProc (void *_workerThread);

public:
  /* Scheduler interface */

    Ref<Scheduler::TaskRecord> scheduleTask (const CallbackDesc<TaskCallback> &cb);

    void cancelTask (Scheduler::TaskRecord *taskRecord);

  /* (End of Scheduler interface) */

    void setMaxThreads (unsigned long maxThreads);

    void setMinThreads (unsigned long minThreads);

    ThreadPool (unsigned long minThreads,
		unsigned long maxThreads)
	 throw (InternalException);

    ~ThreadPool ();
};

}

#endif /* __MYCPP__THREAD_POOL_H__ */

