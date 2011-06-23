#include <mycpp/waitable.h>

namespace MyCpp {

void
Waitable::activeInform (ActiveCallback  callback,
			void           *callbackData,
			void           * /* data */)
{
    callback (callbackData);
}

void
Waitable::fireActive ()
{
    activeInformer->informAll (NULL);
}

Waitable::Waitable ()
{
    activeInformer = grab (new Informer<ActiveCallback> (activeInform));
}

}

