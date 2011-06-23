#ifndef __MYCPP__THREAD_H__
#define __MYCPP__THREAD_H__

#include <mycpp/types.h>
#include <mycpp/simply_referenced.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

/*c*/
class Thread : public virtual SimplyReferenced
{
public:
    /*t*/
    typedef void (*ThreadFunc) (void *data);

protected:
    // GThread*
    void *thread;
    bool  joinable;

    Ref<SimplyReferenced> thread_ref_data;

public:
    // These fields are made public as a workaround
    // to avoid including "glib.h" header file in here.
    // One should not use these from external code.
    ThreadFunc  threadFunc;
    void       *threadFuncData;

public:
    /*m*/
    // TODO throw (InternalException)
    void join ();

    /*m*/
    Thread (ThreadFunc        threadFunc,
	    void             *threadFuncData,
	    SimplyReferenced *ref_data,
	    bool              joinable)
     throw (InternalException);
};

}

#endif /* __MYCPP__THREAD_H__ */

