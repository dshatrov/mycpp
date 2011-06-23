#include <mycpp/trigger.h>

namespace MyCpp {

void
Trigger::eventInform (EventCallback  callback,
		      void          *callbackData,
		      void          * /* _self */)
{
//    Trigger *self = static_cast <Trigger*> (_self);

    callback (callbackData);
}

Trigger::Trigger ()
{
    eventInformer = grab (new Informer<EventCallback> (eventInform));
}

}

