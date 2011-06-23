#ifndef __MYCPP__WAITABLE_H__
#define __MYCPP__WAITABLE_H__

#include <mycpp/object.h>
#include <mycpp/informer.h>

namespace MyCpp {

/*c Abstract waitable object.
 *
 * Waitable objects may be in one of two states: active or inactive.
 * One can wait for an object to become active.
 */
class Waitable : public virtual Object
{
public:
    typedef void (*ActiveCallback) (void *user_data);

protected:
    Ref< Informer<ActiveCallback> > activeInformer;

    static void activeInform (ActiveCallback  callback,
			      void           *callbackData,
			      void           *data);

    void fireActive ();

public:
    virtual bool isActive () = 0;

    // TODO isActive_subscribe()

    Ref< Informer<ActiveCallback> > getActiveInformer ()
    {
	return activeInformer;
    }

    Waitable ();
};

}

#endif /* __MYCPP__WAITABLE_H__ */

