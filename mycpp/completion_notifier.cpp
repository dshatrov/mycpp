#include <mycpp/completion_notifier.h>

namespace MyCpp {

void
CompletionNotifier::_raiseException ()
{
    Ref<Exception> tmp_exc;

    {
      MutexLock state_lock (state_mutex);

      tmp_exc = exc;
    }

    if (!tmp_exc.isNull ())
	tmp_exc->raise ();
}

bool
CompletionNotifier::isComplete ()
{
    bool ret;

    {
      MutexLock state_lock (state_mutex);

	ret = complete;
    }

    return ret;
}

bool
CompletionNotifier::isComplete_setCallback (Referenced *obj,
					    const CallbackDesc<CompletionCallback> *new_cb)
{
    bool ret;

    CallbackDesc<CompletionCallback> tmp_cb;
    {
      MutexLock state_lock (state_mutex);

	tmp_cb = cb;

	weak_obj = obj;
	cb.setDesc (new_cb);

	ret = complete;
    }
    tmp_cb.reset ();

    return ret;
}

void
CompletionNotifier::setCallback (Referenced *obj,
				 const CallbackDesc<CompletionCallback> *new_cb)
{
    CallbackDesc<CompletionCallback> tmp_cb;
    {
      MutexLock state_lock (state_mutex);

	tmp_cb =cb;

	weak_obj = obj;
	cb.setDesc (new_cb);
    }
    tmp_cb.reset ();
}

void
CompletionNotifier::notify (Exception *e)
{
    state_mutex.lock ();

    exc = e;

    CallbackDesc<CompletionCallback> tmp_cb = cb;
    cb.setDesc (NULL);
    complete = true;

    Ref<Referenced> obj;
    if (weak_obj.isValid ()) {
	obj = weak_obj.getRef ();
	if (obj.isNull ()) {
	    state_mutex.unlock ();
	    return;
	}
    }

    if (tmp_cb.refCallback != NULL)
	tmp_cb.refCallback (tmp_cb.refData);

    state_mutex.unlock ();

    if (tmp_cb.callback != NULL)
	tmp_cb.callback (tmp_cb.callbackData);

    if (tmp_cb.unrefCallback != NULL)
	tmp_cb.unrefCallback (tmp_cb.refData);

    obj = NULL;
    tmp_cb.reset ();
}

void
CompletionNotifier::reset ()
{
  MutexLock state_lock (state_mutex);

    complete = false;
    exc = NULL;
}

CompletionNotifier::CompletionNotifier ()
{
    complete = false;
}

CompletionNotifier::CompletionNotifier (Referenced *obj,
					const CallbackDesc<CompletionCallback> *new_cb)
{
    complete = false;
    weak_obj = obj;
    cb.setDesc (new_cb);
}

}

