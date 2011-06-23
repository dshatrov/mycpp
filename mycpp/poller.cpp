#include <mycpp/types.h>
#include <mycpp/active_poll_group.h>
#include <mycpp/simple_threaded_poller.h>

#ifdef PLATFORM_WIN32
#include <mycpp/wsa_poll_group.h>
#endif

#include <mycpp/poller.h>

namespace MyCpp {

Ref<Poller>
Poller::createDefault ()
    throw (InternalException)
{
    // FIXME This is ugly. Think on where the slots_per_thread constant should be taken from.
#ifdef PLATFORM_WIN32
    // TEST
    return grab (static_cast <Poller*> (new SimpleThreadedPoller (WsaPollGroup::getFactory (), 1)));
//    return grab (static_cast <Poller*> (new SimpleThreadedPoller (WsaPollGroup::getFactory (), WsaPollGroup::getMaxWaitObjects ())));
#else
    // Note: SimpleThreadedPoller + EpollPollGroup factory = "too many open files" for small per-thread limits.

    // TEST
//    return grab (static_cast <Poller*> (new SimpleThreadedPoller (getDefaultFactory_ActivePollGroup (), 1)));
    return grab (static_cast <Poller*> (new SimpleThreadedPoller (getDefaultFactory_ActivePollGroup (), MYCPP_SIZE_MAX)));
#endif
}

}

