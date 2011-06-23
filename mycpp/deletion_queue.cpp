#include <mycpp/mycpp_thread_local.h>
#include <mycpp/io.h>

#include <mycpp/deletion_queue.h>

namespace MyCpp {

static List< Ref<SimplyReferenced> >&
getDqueue ()
{
    return myCpp_getThreadLocal ()->deletion_queue;
}

void
deletionQueue_append (SimplyReferenced *obj)
{
    getDqueue ().append (obj);
}

void
deletionQueue_process ()
{
    // TODO Probably the best thing to do would be to avoid recursive calls
    // of deletionQueue_process() to decrease stack pressure.

    // Can't use List::clear(), because deletionQueue_process() may be called
    // recursively (from destructors which lock and unlock 'DataMutex'es).
    List< Ref<SimplyReferenced> > &dqueue = getDqueue ();
    while (dqueue.first != NULL)
	dqueue.remove (dqueue.first);
}

bool
deletionQueue_isEmpty ()
{
    return getDqueue ().first == NULL;
}

}

