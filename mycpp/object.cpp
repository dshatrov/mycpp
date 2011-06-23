#include <mycpp/object.h>

namespace MyCpp {

#if 0
NO DISPOSE
void
Object::_dispose ()
{
    DisposeSlave *oldPtr;
    void *newPtr;

    for (;;) {
	for (;;) {
	    oldPtr = (DisposeSlave*) (firstDisposeSlave.get ());
	    if (oldPtr == NULL)
		break;

	    newPtr = oldPtr->next;
	    if (!firstDisposeSlave.compareAndExchange (oldPtr, newPtr))
		continue;

	    break;
	}

	if (oldPtr == NULL)
	    break;

	delete oldPtr;
    }

    DisposeRecord *cur = firstDisposeRecord;
    while (cur) {
	cur->disposeHandler (cur->data);
	cur = cur->next;
    }
}

void
Object::addDisposeHandler (DisposeHandler  handler,
			   void           *data)
{
    DisposeRecord *dr = new DisposeRecord;

    dr->disposeHandler = handler;
    dr->data = data;
    dr->next = firstDisposeRecord;

    firstDisposeRecord = dr;
}

void
Object::addDisposeSlave (SimplyReferenced *slave)
{
    DisposeSlave *ds = new DisposeSlave;
    void *oldPtr;

    ds->ref = slave;

    for (;;) {
	oldPtr = firstDisposeSlave.get ();
	ds->next = (DisposeSlave*) oldPtr;
	if (!firstDisposeSlave.compareAndExchange (oldPtr, (void*) ds))
	    continue;
	break;
    }
}

Object::~Object ()
{
    DisposeRecord *cur = firstDisposeRecord,
		  *tmp;
    while (cur != NULL) {
	tmp = cur;
	cur = cur->next;
	delete tmp;
    }
}
#endif

}

