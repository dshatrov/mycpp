#ifndef __MYCPP_SYNC_POLLABLE_H__
#define __MYCPP_SYNC_POLLABLE_H__

#include <mycpp/pollable.h>
#include <mycpp/mutex.h>

namespace MyCpp {

class SyncPollable : public Pollable,
		     public virtual Object
{
protected:
    Ref<Pollable> pollable;

    bool inputDirty,
	 outputDirty,
	 errorDirty;

    bool processingInput,
	 processingOutput,
	 processingError;

    Mutex stateMutex;

    /* Must be called with stateMutex held. */
    void doInput  ();

    /* Must be called with stateMutex held. */
    void doOutput ();

    /* Must be called with stateMutex held. */
    void doError  ();

public:
    /* Pollable interface */
    int getFd ();

    void processInput  ();
    void processOutput ();
    void processError  ();
    /* (End of Pollable interface) */

    SyncPollable (Pollable *pollable);
};

}

#endif /* __MYCPP_SYNC_POLLABLE_H__ */

