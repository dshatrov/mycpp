//#include <glib/gthread.h>
#include <glib.h>

#include <mycpp/mycpp_thread_local.h>
#include <mycpp/deletion_queue.h>
#include <mycpp/io.h>

#include <mycpp/cond.h>

namespace MyCpp {

void
CondBase::signal ()
{
    g_cond_signal (static_cast <GCond*> (cond));
}

void
CondBase::broadcast ()
{
    g_cond_broadcast (static_cast <GCond*> (cond));
}

void
CondBase::wait (Mutex &mutex)
{
    g_cond_wait (static_cast <GCond*> (cond),
		 static_cast <GMutex*> (mutex.get_glib_mutex ()));
}

void
CondBase::wait (DataMutex &mutex)
{
    MyCpp_ThreadLocal *tlocal = myCpp_getThreadLocal ();
    if (tlocal->data_mutex_counter == 0)
	abortIfReached ();

    if (!deletionQueue_isEmpty ())
	errf->print ("MyCpp.Cond.wait (DataMutex&): WARNING: deletion queue is not empty")
	     .pendl ();

    g_cond_wait (static_cast <GCond*> (cond),
		 static_cast <GMutex*> (mutex.get_glib_mutex ()));
}

/*
bool timedWait (Mutex    &mutex,
		GTimeVal *tv)
{
    return g_cond_timed_wait (cond, mutex.get_glib_mutex (), tv);
}
*/

CondBase::CondBase ()
{
    if (!gthreadInitialized ())
	abortIfReached ();

    cond = static_cast <void*> (g_cond_new ());
}

CondBase::~CondBase ()
{
    g_cond_free (static_cast <GCond*> (cond));
}

}

