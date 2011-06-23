#include <glib/gthread.h>
#include <glib/gmessages.h>

#include <mycpp/base_util.h>

#include <mycpp/mutex.h>

namespace MyCpp {

void
MutexBase::lock ()
{
    g_mutex_lock (static_cast <GMutex*> (gmutex));
    locked.set (1);
}

// FIXME erdoc: duplicate comment
/*m Unlocks this mutex. */
void
MutexBase::unlock ()
{
    locked.set (0);
    g_mutex_unlock (static_cast <GMutex*> (gmutex));
}

bool
MutexBase::isLocked ()
{
    return locked.get () ? true : false;
}

void
MutexBase::assertLocked ()
{
    abortIf (!isLocked ());
}

void
MutexBase::assertUnlocked ()
{
    abortIf (isLocked ());
}

/* For internal use only:
 * should not be expected to be present in future versions */
void*
MutexBase::get_glib_mutex ()
{
    return gmutex;
}

MutexBase::MutexBase ()
{
    if (!gthreadInitialized ())
	abortIfReached ();

    locked.set (0);
    gmutex = static_cast <void*> (g_mutex_new ());
}

MutexBase::~MutexBase ()
{
    if (locked.get ()) {
	g_critical ("MyCpp.MutexBase~(): locked");
	abortIfReached ();
    }

    g_mutex_free (static_cast <GMutex*> (gmutex));
}

}

