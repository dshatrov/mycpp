#ifndef __MYCPP_CALL_H__
#define __MYCPP_CALL_H__

#include <mycpp/object.h>
#include <mycpp/waitable.h>

namespace MyCpp {

/* 08.03.08
 * NOTE: This interface is DEPRECATED and should not be used anymore.
 * It used to serve as an abstraction for deferred procedure calls
 * in MyRelay, but now it's postulated that no call can be cancelled
 * after being issued, and it is clear that it would be more practical
 * to leave implementation of blocking to the caller's side. */

class Call : public Waitable,
	     public virtual Object
{
public:
    virtual void block  () = 0;
    virtual void cancel () = 0;
};

}

#endif /* __MYCPP_CALL_H__ */

