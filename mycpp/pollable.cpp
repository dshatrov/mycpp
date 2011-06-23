#include <mycpp/io.h>
#include <mycpp/pollable.h>

namespace MyCpp {

void
IoActor_Closing::preClosedInform (PreClosedCallback callback,
				  void *callback_data,
				  void * /* data */)
{
    callback (callback_data);
}

void
IoActor_Closing::ioActor_doPreClose ()
{
    state_mutex.lock ();
    if (pre_closed) {
	state_mutex.unlock ();
	return;
    }

    pre_closed = true;
    state_mutex.unlock ();

    firePreClosed ();
}

bool
IoActor_Closing::isPreClosed_subscribe (CallbackDesc<PreClosedCallback> const *cb,
					unsigned long flags,
					Ref<GenericInformer::Subscription> *ret_sbn)
{
    if (ret_sbn != NULL)
	*ret_sbn = NULL;

    {
      MutexLock state_lock (state_mutex);

	if (pre_closed)
	    return true;

	if (cb != NULL) {
	    if (ret_sbn != NULL)
		*ret_sbn = getPreClosedInformer ()->subscribe (*cb, flags);
	    else
		getPreClosedInformer ()->subscribe (*cb, flags);
	}
    }

    return false;
}

IoActor_Closing::IoActor_Closing ()
{
    pre_closed_informer = grab (new Informer<PreClosedCallback> (preClosedInform));
}

IoActor_Closing::~IoActor_Closing ()
{
    state_mutex.lock ();
    if (!pre_closed) {
	state_mutex.unlock ();
	errf->print ("MyCpp.IoActor_Closing.~(): WARNING: doPreClose has not been called").pendl ();
    } else
	state_mutex.unlock ();
}

}

