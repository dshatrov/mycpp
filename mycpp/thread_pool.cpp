#include <mycpp/thread_pool.h>
#include <mycpp/io.h>

namespace MyCpp {

void
ThreadPool::WorkerThread::workerThreadProc (void *_self)
{
    WorkerThread *self = static_cast <WorkerThread*> (_self);

    for (;;) {
	self->activeMutex.lock ();

	while (!self->active)
	    self->activeCond.wait (self->activeMutex);

	if (self->shouldStop) {
	    self->activeMutex.unlock ();
	    return;
	}

	self->active = false;

	self->activeMutex.unlock ();

	ThreadPool::workerProc (self);
    }
}

void
ThreadPool::WorkerThread::resume ()
{
    activeMutex.lock ();
    active = true;
    activeCond.signal ();
    activeMutex.unlock ();
}

void
ThreadPool::WorkerThread::stop ()
{
    activeMutex.lock ();
    shouldStop = true;
    active = true;
    activeCond.signal ();
    activeMutex.unlock ();
}

void
ThreadPool::WorkerThread::join ()
{
    thread->join ();
}

ThreadPool::WorkerThread::WorkerThread (ThreadPool *thread_pool)
    throw (InternalException)
{
    this->thread_pool = thread_pool;

    /* Throws InternalException */
    thread = grab (new Thread (workerThreadProc,
			       (WorkerThread*) this,
			       NULL,
			       true /* joinable */));
}

// Must be called with 'state_mutex' held.
void
ThreadPool::spawnSpareThreads ()
    throw (InternalException)
{
    unsigned long nthreads = spareThreads.getNumElements () +
			     busyThreads.getNumElements ();

    unsigned long tocreate = 0;

    if (nthreads < minThreads)
	tocreate = minThreads - nthreads;

    Ref<WorkerThread> workerThread;
    for (unsigned long i = 0; i < tocreate; i++) {
	try {
	    /* Throws InternalException */
	    workerThread = grab (new WorkerThread (this));

	    spareThreads.append (workerThread);
	} catch (Exception &exc) {
	    if (spareThreads.getNumElements () + busyThreads.getNumElements () == 0) {
		throw InternalException (
			String::forPrintTask (
				(Pr ("MyCpp.ThreadPool.spawnSpareThreads: "
				     "failed to spawn the first thread: "))
				(Pr (exc.getMessage ()))),
			exc.clone ());
	    } else {
		errf->print ("MyCpp.ThreadPool.spawnSpareThreads: "
			     "WARNING: failed to spawn a thread");
	    }
	}
    }
}

// Must be called with 'state_mutex' held.
void
ThreadPool::activateWorkerThread (WorkerThread *workerThread,
				  TaskRecord   *taskRecord)
{
    if (taskRecord == NULL ||
	!taskRecord->valid)
    {
	// Sanity check
	abortIfReached ();
    }

    busyThreads.append (workerThread, busyThreads.last);

    workerThread->storedLink = busyThreads.last;
    workerThread->storedTask = taskRecord;

    workerThread->resume ();
}

void
ThreadPool::workerProc (void *_workerThread)
{
    WorkerThread *workerThread = static_cast <WorkerThread*> (_workerThread);
    ThreadPool *self = workerThread->thread_pool;

    self->state_mutex.lock ();

    if (!workerThread->storedTask.isNull ()) {
	Ref<TaskRecord> const &tr = workerThread->storedTask;

	if (tr->valid) {
	    tr->valid = false;

	    bool skip = false;
	    Ref<Referenced> obj;
	    if (tr->cb.weak_obj.isValid ()) {
		obj = tr->cb.weak_obj.getRef ();
		if (obj.isNull ())
		    skip = true;
	    }

	    if (!skip) {
		if (tr->cb.refCallback != NULL)
		    tr->cb.refCallback (tr->cb.refData);

		self->state_mutex.unlock ();

		if (tr->cb.callback != NULL)
		    tr->cb.callback (tr->cb.callbackData);

		if (tr->cb.unrefCallback != NULL)
		    tr->cb.unrefCallback (tr->cb.refData);

		self->state_mutex.lock ();
	    }
	}

	workerThread->storedTask = NULL;
    }

    while (self->pendingTasks.first != NULL) {
	Ref<TaskRecord> tr = self->pendingTasks.first->data;
	if (!tr->valid)
	    abortIfReached ();

	self->pendingTasks.remove (self->pendingTasks.first);
	tr->tasksLink = NULL;
	tr->valid = false;

	bool skip = false;
	Ref<Referenced> obj;
	if (tr->cb.weak_obj.isValid ()) {
	    obj = tr->cb.weak_obj.getRef ();
	    if (obj.isNull ())
		skip = true;
	}

	if (!skip) {
	    if (tr->cb.refCallback != NULL)
		tr->cb.refCallback (tr->cb.refData);

	    self->state_mutex.unlock ();

	    if (tr->cb.callback != NULL)
		tr->cb.callback (tr->cb.callbackData);

	    if (tr->cb.unrefCallback != NULL)
		tr->cb.unrefCallback (tr->cb.refData);

	    self->state_mutex.lock ();
	}
    }

    if (self->spareThreads.getNumElements () +
	self->busyThreads.getNumElements ()
		<= self->minThreads)
    {
	self->spareThreads.append (workerThread, self->spareThreads.last);
    }

    if (workerThread->storedLink != NULL) {
	self->busyThreads.remove (workerThread->storedLink);
	workerThread->storedLink = NULL;
    }

    self->state_mutex.unlock ();
}

void
ThreadPool::setMaxThreads (unsigned long maxThreads)
{
    state_mutex.lock ();

    this->maxThreads = maxThreads;

    unsigned long nthreads = spareThreads.getNumElements () +
			     busyThreads.getNumElements ();

    while (nthreads < maxThreads &&
	   pendingTasks.first != NULL)
    {
	Ref<WorkerThread> workerThread;

	try {
	    /* Throws InternalException */
	    workerThread = grab (new WorkerThread (this));
	} catch (Exception &exc) {
	    /* Failure to spawn an excessive thread is not fatal. */
	    errf->print ("MyCpp.ThreadPool.scheduleTask: "
			 "failed to create a new thread: ")
		 .print (exc.getMessage ())
		 .pendl ();
	    break;
	}

	activateWorkerThread (workerThread,
			      pendingTasks.first->data);

	pendingTasks.first->data->tasksLink = NULL;
	pendingTasks.remove (pendingTasks.first);

	nthreads ++;
    }

    state_mutex.unlock ();
}

void
ThreadPool::setMinThreads (unsigned long minThreads)
{
    List< Ref<WorkerThread> > threads_to_join;

    state_mutex.lock ();

    this->minThreads = minThreads;

    unsigned long nthreads = spareThreads.getNumElements () +
			     busyThreads.getNumElements ();

    if (minThreads < nthreads) {
	unsigned long toremove = nthreads - minThreads,
		      i;

	for (i = 0; i < toremove; i++) {
	    if (spareThreads.last == NULL)
		break;

	    spareThreads.last->data->stop ();
	    threads_to_join.append (spareThreads.last->data);
	    spareThreads.remove (spareThreads.last);

	    // TODO 08.07.05: Right now, shrinking is opportunistic. For example,
	    // if all threads are busy at the moment setMinThreads() is called,
	    // then no threads will be terminated at all. This is because the threads
	    // are joinable, and we don't want to wait for an opportunity to join
	    // a now-busy thread (can't do that, because it may never become spare).
	}
    } else {
	try {
	    /* Throws InternalException */
	    spawnSpareThreads ();
	} catch (Exception &exc) {
	    /* This place is effectively unreachable. spawnSpareThreads()
	     * can throw an InternalException only for the
	     * first run, which is performed in the ThreadPool() constructor. */
	    abortIfReached ();
	}
    }

    state_mutex.unlock ();

    {
	List< Ref<WorkerThread> >::DataIterator threads_iter (threads_to_join);
	while (!threads_iter.done ())
	    threads_iter.next ()->join ();
    }
}

Ref<Scheduler::TaskRecord>
ThreadPool::scheduleTask (const CallbackDesc<TaskCallback> &cb)
{
    state_mutex.lock ();

    Ref<WorkerThread> workerThread;

    if (spareThreads.first != NULL) {
	workerThread = spareThreads.first->data;
	spareThreads.remove (spareThreads.first);
    } else {
	unsigned long nthreads = spareThreads.getNumElements () +
				 busyThreads.getNumElements ();

	if (nthreads < maxThreads) {
	    try {
		/* Throws InternalException */
		workerThread = grab (new WorkerThread (this));
	    } catch (Exception &exc) {
		/* Failure to spawn an excessive thread is not fatal. */
		errf->print ("MyCpp.ThreadPool.scheduleTask: "
			     "failed to create a new thread: ")
		     .print (exc.getMessage ())
		     .pendl ();
	    }
	}
    }

    Ref<TaskRecord> taskRecord = grab (new TaskRecord);
    taskRecord->valid = true;
    taskRecord->cb.setDesc (&cb);

    if (!workerThread.isNull ()) {
	taskRecord->tasksLink = NULL;
	activateWorkerThread (workerThread, taskRecord);
    } else {
	taskRecord->tasksLink = pendingTasks.append (taskRecord);
    }

    state_mutex.unlock ();

//    return takeOver<Scheduler::TaskRecord> (taskRecord);
    return taskRecord.ptr ();
}

void
ThreadPool::cancelTask (Scheduler::TaskRecord *_taskRecord)
{
    TaskRecord *taskRecord = static_cast <TaskRecord*> (_taskRecord);

    state_mutex.lock ();
    if (taskRecord->valid) {
	taskRecord->valid = false;
	if (taskRecord->tasksLink != NULL) {
	    pendingTasks.remove (taskRecord->tasksLink);
	    taskRecord->tasksLink = NULL;
	}
    }
    state_mutex.unlock ();
}

ThreadPool::ThreadPool (unsigned long minThreads,
			unsigned long maxThreads)
    throw (InternalException)
{
    if (minThreads == 0) {
	errf->print ("MyCpp.ThreadPool(): minThreads is 0, forced to be 1")
	     .pendl ();
	minThreads = 1;
    }

    if (maxThreads == 0) {
	errf->print ("MyCpp.ThreadPool(): maxThreads is 0, forced to be 1")
	     .pendl ();
	maxThreads = 1;
    }

    this->maxThreads = maxThreads;
    this->minThreads = minThreads;

    // Holding the mutex just for kicks.
    state_mutex.lock ();

    try {
	/* Throws InternalException */
	spawnSpareThreads ();
    } catch (Exception &exc) {
	state_mutex.unlock ();
	exc.raise ();
    }

    state_mutex.unlock ();
}

ThreadPool::~ThreadPool ()
{
    state_mutex.lock ();

    {
	List< Ref<TaskRecord> >::DataIterator tr_iter (pendingTasks);
	while (!tr_iter.done ()) {
	    Ref<TaskRecord> &tr = tr_iter.next ();
	    tr->valid = false;
	    tr->tasksLink = NULL;
	}
	pendingTasks.clear ();
    }

    List< Ref<WorkerThread> > threads_to_join;
    threads_to_join.steal (&busyThreads, busyThreads.first, busyThreads.last,
			   threads_to_join.last, GenericList::StealAppend);
    threads_to_join.steal (&spareThreads, spareThreads.first, spareThreads.last,
			   threads_to_join.last, GenericList::StealAppend);
    {
	List< Ref<WorkerThread> >::DataIterator threads_iter (threads_to_join);
	while (!threads_iter.done ()) {
	    Ref<WorkerThread> &worker_thread = threads_iter.next ();
	    worker_thread->storedLink = NULL;
	    worker_thread->stop ();
	}
    }

    state_mutex.unlock ();

    {
	List< Ref<WorkerThread> >::DataIterator threads_iter (threads_to_join);
	while (!threads_iter.done ())
	    threads_iter.next ()->join ();
    }
}

}

