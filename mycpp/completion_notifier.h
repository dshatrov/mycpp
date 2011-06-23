#ifndef __MYCPP__COMPLETION_NOTIFIER_H__
#define __MYCPP__COMPLETION_NOTIFIER_H__

#include <mycpp/object.h>
#include <mycpp/callback_desc.h>
#include <mycpp/mutex.h>
#include <mycpp/exception.h>

namespace MyCpp {

class CompletionNotifier : public virtual SimplyReferenced
{
public:
    typedef void (*CompletionCallback) (void *data);

protected:
    WeakRef<Referenced> weak_obj;
    CallbackDesc<CompletionCallback> cb;

    bool complete;
    Ref<Exception> exc;

    Mutex state_mutex;

protected:
    void _raiseException ();

public:
    bool isComplete ();

    bool isComplete_setCallback (Referenced *obj,
				 const CallbackDesc<CompletionCallback> *new_cb);

    void setCallback (Referenced *obj,
		      const CallbackDesc<CompletionCallback> *new_cb);

    void notify (Exception *e);

    void reset ();

    CompletionNotifier ();

    CompletionNotifier (Referenced *obj,
			const CallbackDesc<CompletionCallback> *new_cb);
};

}

#endif /* __MYCPP__COMPLETION_NOTIFIER_H__ */

