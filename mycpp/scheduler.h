#ifndef __MYCPP_SCHEDULER_H__
#define __MYCPP_SCHEDULER_H__

#include <mycpp/types.h>
#include <mycpp/object.h>
#include <mycpp/callback_desc.h>

namespace MyCpp {

class Scheduler : public virtual Object
{
public:
    typedef void (*TaskCallback) (void *userData);

    class TaskRecord : public virtual SimplyReferenced
    {
    };

    virtual Ref<TaskRecord> scheduleTask (const CallbackDesc<TaskCallback> &cb) = 0;

    virtual void cancelTask (TaskRecord *taskRecord) = 0;
};

}

#endif /* __MYCPP_SCHEDULER_H__ */

