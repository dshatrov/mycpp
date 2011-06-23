#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/background_poller.h>

namespace MyCpp {

namespace BackgroundPoller_priv {}
using namespace BackgroundPoller_priv;

namespace BackgroundPoller_priv {

    class MainThreadData : public SimplyReferenced
    {
    public:
      // mt_const

	WeakRef<Poller> weak_poller;

      // (end mt_const)
    };

}

void
BackgroundPoller::main_thread_proc (void *_data)
{
    MainThreadData * const &data = static_cast <MainThreadData*> (_data);

    Ref<Poller> poller = data->weak_poller.getRef ();
    if (poller.isNull ())
	return;

    try {
	poller->run ();
    } catch (Exception &exc) {
	errf->print ("MyCpp.BackgroundPoller.main_thread_proc: ");
	printException (errf, exc);
    }
}

void
BackgroundPoller::runInBackground ()
    throw (InternalException)
{
  DataMutexLock state_lock (state_mutex);

    Ref<MainThreadData> main_thread_data = grab (new MainThreadData);
    main_thread_data->weak_poller = poller;

    main_thread = grab (new Thread (main_thread_proc,
				    main_thread_data,
				    main_thread_data,
				    true /* joinable */));
}

void
BackgroundPoller::stopNoJoin ()
    throw (InternalException)
{
    poller->stopNoJoin ();
}

void
BackgroundPoller::join ()
    throw (InternalException)
{
    poller->join ();

    Ref<Thread> tmp_main_thread;
    {
      DataMutexLock state_lock (state_mutex);

	tmp_main_thread = main_thread;
    }

    if (!tmp_main_thread.isNull ())
	tmp_main_thread->join ();
}

BackgroundPoller::BackgroundPoller (Poller *poller)
{
    abortIf (poller == NULL);
    this->poller = poller;
}

}

