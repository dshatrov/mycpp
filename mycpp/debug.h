#ifndef __MYCPP__DEBUG_H__
#define __MYCPP__DEBUG_H__

#include <mycpp/atomic_int.h>
#include <mycpp/util.h>

namespace MyCpp {

class NonreentrantTest
{
protected:
    AtomicInt &counter;

public:
    NonreentrantTest (AtomicInt &counter)
	: counter (counter)
    {
	int value;
	do {
	    value = counter.get ();
	    if (value != 0)
		abortIfReached ();
	} while (counter.compareAndExchange (value, value + 1));
    }

    ~NonreentrantTest ()
    {
	if (!counter.decAndTest ())
	    abortIfReached ();
    }
};

/* NOTE: Do not use --fno-threadsafe-statics with this code. */
#define MYCPP_NONREENTRANT							\
    static AtomicInt __mycpp_nonreentrant_counter (0);				\
    NonreentrantTest __mycpp_nonreentrant (__mycpp_nonreentrant_counter);

}

#endif /* __MYCPP__DEBUG_H__ */

