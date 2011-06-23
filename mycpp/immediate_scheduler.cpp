#include <mycpp/util.h>
#include <mycpp/io.h> // DEBUG
#include <mycpp/immediate_scheduler.h>

#define DEBUG(a) ;

namespace MyCpp {

void
ImmediateScheduler::triggerEventCallback (void *_self)
{
  DEBUG (
    errf->print ("MyCpp.ImmediateScheduler.triggerEventCallback").pendl ();
  )

    ImmediateScheduler *self = static_cast <ImmediateScheduler*> (_self);

    self->tasks_mutex.lock ();

    if (self->task_queue.last == NULL) {
	self->tasks_mutex.unlock ();
	return;
    }

    Ref<TaskRecord> tr = self->task_queue.last->data;
    /* If TaskRecord is on the queue, then it is surely valid. */
    tr->valid = false;
    self->task_queue.remove (self->task_queue.last);

    bool got_more_tasks = (self->task_queue.first != NULL);

    Ref<Referenced> obj;
    if (tr->deletion_sbn) {
	obj = tr->cb.weak_obj.getRef ();
	if (obj.isNull ()) {
	    self->tasks_mutex.unlock ();

	    // We can get here if clientDeletionCallback() is called
	    // after we took 'tasks_mutex'. We've already invalidated
	    // the task and removed it from the queue, hence the need
	    // to perform cleanups.
	    tr->deletion_sbn = NULL;
	    tr->cb.reset ();

	    if (got_more_tasks)
		self->trigger.trigger ();

	    return;
	}
    }

    if (tr->cb.refCallback != NULL)
	tr->cb.refCallback (tr->cb.refData);

    self->tasks_mutex.unlock ();

    if (got_more_tasks) {
	// We're in triggerEventCallback(), hence this will be 
	// posponed till the end of this method at least.
	self->trigger.trigger ();
    }

    if (tr->cb.callback != NULL)
	tr->cb.callback (tr->cb.callbackData);

    if (tr->cb.unrefCallback != NULL)
	tr->cb.unrefCallback (tr->cb.refData);

    if (!obj.isNull ())
	obj->removeDeletionCallback (tr->deletion_sbn);

    tr->deletion_sbn = NULL;
    tr->cb.reset ();
}

void
ImmediateScheduler::clientDeletionCallback (void *_tr)
{
    TaskRecord *tr = static_cast <TaskRecord*> (_tr);
    ImmediateScheduler *&self = tr->immediate_scheduler;

    self->tasks_mutex.lock ();

    if (!tr->valid) {
	self->tasks_mutex.unlock ();
	return;
    }
    tr->valid = false;

    self->task_queue.remove (tr->scheduler_link);

    self->tasks_mutex.unlock ();

    tr->deletion_sbn = NULL;
    tr->cb.reset ();
}

Ref<Scheduler::TaskRecord>
ImmediateScheduler::scheduleTask (const CallbackDesc<TaskCallback> &cb)
{
  DEBUG (
    errf->print ("MyCpp.ImmediateScheduler.scheduleTask").pendl ();
  )

    Ref<Referenced> obj;
    if (cb.weak_obj.isValid ()) {
	obj = cb.weak_obj.getRef ();
	if (obj.isNull ())
	    return NULL;
    }

    Ref<TaskRecord> tr = grab (new TaskRecord);
    tr->valid = true;
    tr->immediate_scheduler = this;
    tr->cb = cb;

    if (!obj.isNull ()) {
#if 0
	CallbackDesc<Referenced::DeletionCallback> dcb;
	dcb.weak_obj = this;
	dcb.callback = clientDeletionCallback;
	dcb.callbackData = static_cast <void*> (tr);
	dcb.addRefData (tr);

	tr->deletion_sbn = obj->addDeletionCallback (dcb);
#endif
	tr->deletion_sbn = obj->addDeletionCallbackNonmutual (clientDeletionCallback, tr, tr, this);
    }

    tasks_mutex.lock ();
    tr->scheduler_link = task_queue.append (tr);
    tasks_mutex.unlock ();

  DEBUG (
    errf->print ("MyCpp.ImmediateScheduler.scheduleTask: calling trigger()").pendl ();
  )
    trigger.trigger ();

    /* ? Is there any point in this check?
     * We're using an ImmediateTrigger anyway... */
    tasks_mutex.lock ();
    if (!tr->valid)
	tr = NULL;
    tasks_mutex.unlock ();

    return tr.ptr ();
}

void
ImmediateScheduler::cancelTask (Scheduler::TaskRecord *_taskRecord)
{
    if (_taskRecord == NULL)
	return;

    TaskRecord *tr = static_cast <TaskRecord*> (_taskRecord);

    tasks_mutex.lock ();

    if (!tr->valid) {
	tasks_mutex.unlock ();
	return;
    }
    tr->valid = false;

    task_queue.remove (tr->scheduler_link);

    tasks_mutex.unlock ();

    if (tr->deletion_sbn) {
	Ref<Referenced> obj = tr->cb.weak_obj.getRef ();
	if (!obj.isNull ())
	    obj->removeDeletionCallback (tr->deletion_sbn);

	tr->deletion_sbn = NULL;
    }

    tr->cb.reset ();
}

ImmediateScheduler::ImmediateScheduler ()
{
    CallbackDesc<Trigger::EventCallback> cb;
    cb.weak_obj = this;
    cb.callback = triggerEventCallback;
    cb.callbackData = static_cast <void*> (this);

    trigger.getEventInformer ()->subscribe (cb);
}

ImmediateScheduler::~ImmediateScheduler ()
{
    // This lock/unlock pais serves as a memory barrier.
    tasks_mutex.lock ();
    tasks_mutex.unlock ();

    List< Ref<TaskRecord> >::DataIterator tasks_iter (task_queue);
    while (!tasks_iter.done ()) {
	Ref<TaskRecord> &tr = tasks_iter.next ();

	if (!tr->valid)
	    abortIfReached ();

	if (tr->deletion_sbn) {
	    Ref<Referenced> obj = tr->cb.weak_obj.getRef ();
	    if (!obj.isNull ())
		obj->removeDeletionCallback (tr->deletion_sbn);

	    tr->deletion_sbn = NULL;
	}

	tr->cb.reset ();
    }
}

}

