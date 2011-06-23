#ifndef __MYCPP__BACKGROUND_POLER_H__
#define __MYCPP__BACKGROUND_POLER_H__

#include <mycpp/data_mutex.h>
#include <mycpp/object.h>
#include <mycpp/thread.h>
#include <mycpp/poller.h>

namespace MyCpp {

class BackgroundPoller : public virtual Object
{
protected:
  // mt_const

    Ref<Poller> poller;

  // (end mt_const)

  // mt_mutex state_mutex

    Ref<Thread> main_thread;

  // (end mt_mutex state_mutex)

    DataMutex state_mutex;

    static void main_thread_proc (void *_data);

public:
    void runInBackground ()
		   throw (InternalException);

    void stopNoJoin ()
	      throw (InternalException);

    void join ()
	throw (InternalException);

    void stopJoin ()
	    throw (InternalException)
    {
	stopNoJoin ();
	join ();
    }

    BackgroundPoller (Poller *poller);
};

}

#endif /* __MYCPP__BACKGROUND_POLER_H__ */

