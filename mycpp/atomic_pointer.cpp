#include <glib.h>

#include <mycpp/atomic_pointer.h>

namespace MyCpp {

void
AtomicPointer::set (void *ptr)
{
    g_atomic_pointer_set (&this->ptr, (gpointer) ptr);
}

void*
AtomicPointer::get () const
{
    return (void*) g_atomic_pointer_get (&ptr);
}

bool
AtomicPointer::compareAndExchange (void *oldPtr,
				   void *newPtr)
{
    return (bool) g_atomic_pointer_compare_and_exchange (
			&ptr, (gpointer) oldPtr, (gpointer) newPtr);
}

AtomicPointer::AtomicPointer ()
{
    /* Is it so wise to perform a memory-barrier-operation
     * without real neccessity? (this applies to AtomicInt as well) */
    g_atomic_pointer_set (&ptr, NULL);
}

}

