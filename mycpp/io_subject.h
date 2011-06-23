#ifndef __MYCPP__IO_SUBJECT_H__
#define __MYCPP__IO_SUBJECT_H__

#include <mycpp/referenced.h>

namespace MyCpp {

/* This interface has been introduced to avoid re-implementation
 * of setNeedInput() and setNeedOutput() for each class derived
 * from IoActor. The dominating implementation is redirecting
 * setNeed*() calls to appropriate informers from IoActor interface.
 */

class IoSubject : public virtual SimplyReferenced
{
public:
    virtual void setNeedInput (bool need) = 0;

    virtual void setNeedOutput (bool need) = 0;

    void requestInput ()
    {
	setNeedInput (true);
    }

    void requestOutput ()
    {
	setNeedOutput (true);
    }
};

}

#endif /* __MYCPP__IO_SUBJECT_H__ */

