//#include <libmary/libmary.h>
// TODO Separate include for libMaryInit()
namespace M {
    void libMaryInit ();
}

#include <cstdio>

#ifdef PLATFORM_WIN32
#include <Winsock2.h>
#endif

#include <glib.h>
#include <mycpp/mycpp_config.h>
#include <mycpp/object.h>
#include <mycpp/mutex.h>
#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/branch.h>
#include <mycpp/mycpp_thread_local.h>
#ifdef MYCPP_ENABLE_MEMPROF
#include <mycpp/mem_prof.h>
#endif

#ifdef PLATFORM_WIN32
#include <mycpp/win32.h>
#else
#include <mycpp/posix.h>
#endif

// Note: This is just for syntax checking of mycpp.h.
#include <mycpp/mycpp.h>

/* DEBUG
#include <mycpp/dynamic_tree_array.h>
MyCpp::DynamicTreeArray<int> dta (4);
 (DEBUG) */


//#define MYCPP__FAKE_LOCK


namespace MyCpp {

namespace {

    bool mycpp_glob_initialized = false;

}

Ref<File> stdoutFile;
Ref<File> stderrFile;

File *outf;
File *errf;

File *failmodeErrf;
File *failmodeOutf;

PrintTaskFactory Pr;

static Ref< Mutex_<SimplyReferenced> > libraryMutex,
				       helperMutex;
static bool gthread_initialized = false;

void
referencedRefCallback (void *_obj)
{
    SimplyReferenced *obj = static_cast <SimplyReferenced*> (_obj);
    obj->ref ();
}

void
referencedUnrefCallback (void *_obj)
{
    SimplyReferenced *obj = static_cast <SimplyReferenced*> (_obj);
    obj->unref ();
}

void
myCppInit (void)
{
    {
      // Protecting from multiple invocations of myCppInit().

	if (mycpp_glob_initialized)
	    return;

	mycpp_glob_initialized = true;
    }

#ifdef PLATFORM_WIN32
    {
	// NOTE: We never call WSACleanup()...
	WORD wsa_version = MAKEWORD (2, 2);
	WSADATA wsa_data;
	if (WSAStartup (wsa_version, &wsa_data)) {
	    fprintf (stderr, "FATAL: WSAStartup() failed\n");
	    fflush (stderr);
	    abortIfReached ();
	}
    }
#endif

    // Introduced for gstreamer initialization
    // (gstreamer initializes gthread silently).
// TODO ENABLE THIS AFTER UPGRADING GLIB
//    if (!g_thread_get_initialized ()) {
	g_thread_init (NULL /* vtable */);
//    }
    gthread_initialized = true;

    M::libMaryInit ();

#ifdef MYCPP_ENABLE_MEMPROF
    // MemProf _must_ be initialized first.
    memProfInit ();
#endif

    libraryMutex = grab (new Mutex_<SimplyReferenced>);
    helperMutex = grab (new Mutex_<SimplyReferenced>);

#ifdef PLATFORM_WIN32
    win32_myCppInit ();
#else
    posix_myCppInit ();
#endif

    threadLocal_myCppInit ();

    // Lock/unlock a mutex to ensure a "memory barrier" for static data
    // (this just looks like a good idea).
    libraryMutex->lock ();
    libraryMutex->unlock ();
}

bool
gthreadInitialized ()
{
    return gthread_initialized;
}

void
libraryLock ()
{
#ifndef MYCPP__FAKE_LOCK
    if (libraryMutex.isNull ())
	abortIfReached ();

    libraryMutex->lock ();
#endif
}

void
libraryUnlock ()
{
#ifndef MYCPP__FAKE_LOCK
    if (libraryMutex.isNull ())
	abortIfReached ();

    libraryMutex->unlock ();
#endif
}

void
helperLock ()
{
#ifndef MYCPP__FAKE_LOCK
    if (helperMutex.isNull ())
	abortIfReached ();

    helperMutex->lock ();
#endif
}

void
helperUnlock ()
{
#ifndef MYCPP__FAKE_LOCK
    if (helperMutex.isNull ())
	abortIfReached ();

    helperMutex->unlock ();
#endif
}

}

