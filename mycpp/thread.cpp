#include <glib.h>

#include <mycpp/io.h>
#include <mycpp/util.h>

#include <mycpp/thread.h>

namespace MyCpp {

static gpointer
wrapperThreadFunc (gpointer data)
{
    Thread *self = (Thread*) data;

    try {
	self->threadFunc (self->threadFuncData);
    } catch (...) {
	errf->print ("MyCpp.Thread.wrapperThreadFunc: "
		     "WARNING: unhandled exception").pendl ();
    }

    self->unref ();

    return (gpointer) 0;
}

void
Thread::join ()
{
    if (!joinable) {
	errf->print ("MyCpp.Thread.join: "
		     "WARNING: attempted to join a non-joinable thread").pendl ();
	return;
    }

    g_thread_join (static_cast <GThread*> (thread));

    // NOTE: Better to comment this out, because it's not clear whether g_thread_join()
    // may fail while wrapperThreadFunc() is still running. Keeping 'thread_ref_data'
    // for the duration of 'Thread's lifetime shouldn't do any harm.
    //
    // thread_ref_data = NULL;
}

Thread::Thread (ThreadFunc        threadFunc,
		void             *threadFuncData,
		SimplyReferenced *ref_data,
		bool              joinable)
    throw (InternalException)
{
    if (!gthreadInitialized ())
	abortIfReached ();

    this->joinable = joinable;

    this->threadFunc = threadFunc;
    this->threadFuncData = threadFuncData;
    this->thread_ref_data = ref_data;

    this->ref ();

    GError *error = NULL;
    thread = static_cast <void*> (g_thread_create (wrapperThreadFunc,
						   this,
						   joinable ? TRUE : FALSE,
						   &error));
    if (thread == NULL) {
	InternalException exc (String::forData (error->message));
	g_clear_error (&error);

	this->unref ();

	throw exc;
    }
}

}

