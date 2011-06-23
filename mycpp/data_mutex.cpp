#include <mycpp/mycpp_thread_local.h>
#include <mycpp/deletion_queue.h>
#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/data_mutex.h>

/* DEBUG
#include <mycpp/internal_exception.h>
#include <mycpp/io.h>
 (DEBUG) */

namespace MyCpp {

void
DataMutexBase::lock ()
{
    mutex.lock ();
    {
	MyCpp_ThreadLocal *tlocal = myCpp_getThreadLocal ();

// TODO Implement class DataMutexForbidden, see the comment below.

/* This check can't withstand critics. Whether a method is allowed to be called
 * with a DataMutex held or not is decided separately for each method. This
 * means nothing more but that a special helper-object can be introduced
 * to catch invalid invocations (with a name like "DataMutexForbidden").
	if (tlocal->data_mutex_counter >= 1)
	    abortIfReached ();
*/

	tlocal->data_mutex_counter ++;
    }

  /* DEBUG
    try {
	throw InternalException ();
    } catch (Exception &exc) {
	errf->print ("MyCpp.DataMutex.lock: backtrace:\n")
	     .print (exc.getBacktrace ())
	     .pendl ();
    }
   (DEBUG) */
}

void
DataMutexBase::unlock ()
{
    {
	MyCpp_ThreadLocal *tlocal = myCpp_getThreadLocal ();
	abortIf (tlocal->data_mutex_counter == 0);
	tlocal->data_mutex_counter --;
	if (tlocal->data_mutex_counter == 0) {
	    mutex.unlock ();

	    deletionQueue_process ();
	    return;
	}
    }
    mutex.unlock ();
}

void
DataMutexBase::assertLocked ()
{
    mutex.assertLocked ();
}

void
DataMutexBase::assertUnlocked ()
{
    mutex.assertUnlocked ();
}

void*
DataMutexBase::get_glib_mutex ()
{
    return mutex.get_glib_mutex ();
}

}

