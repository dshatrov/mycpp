#include <cstdio>

#include <mycpp/simply_referenced.h>

#define DEBUG(a) ;

namespace MyCpp {

AtomicInt SimplyReferenced::num_objects;

void
SimplyReferenced::unref ()
{
  DEBUG (
    if (traced.get ())
	fprintf (stderr, "reftrace: unref: %lx\n", (unsigned long) this);
  ) /* (DEBUG) */

    int old;

    /* TODO FIXME The statement below is false. You should have been more
     * careful with borrowing stuff. sub_last_unref() CAN be called concurrently
     * here, hence the current implementation of Referenced::sub_last_unref() is
     * inadequate.
     */
    /* The point here is to call _sub_last_unref() BEFORE removing
     * the last reference. If we detect that a reference being
     * removed is the last one, then no one else will assume
     * his reference to be the last until we actually remove
     * our last reference. Provided that, it is guaranteed
     * that no concurrent calls to _sub_last_unref() will be made.
     *
     * The technique was borrowed from glib
     * (glib-2.12.4, g_object_unref(), gobject/gobject.c:1731). */
    for (;;) {
	old = refCount.get ();
	if (old > 1) {
	    if (!refCount.compareAndExchange (old, old - 1))
		continue;

	    return;
	}

	break;
    }

    if (sub_last_unref ())
	delete this;
}

}

