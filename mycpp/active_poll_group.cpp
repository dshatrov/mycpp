#include <mycpp/util.h>

#include <mycpp/active_poll_group.h>
#include <mycpp/busy_poll_group.h>

#ifdef PLATFORM_WIN32
#include <mycpp/wsa_poll_group.h>
#else
#include <mycpp/epoll_poll_group.h>
#include <mycpp/select_poll_group.h>
#endif

namespace MyCpp {

Ref<ActivePollGroup>
ActivePollGroup::createDefault ()
    throw (InternalException)
{
#ifdef PLATFORM_WIN32
//    return grab (static_cast <ActivePollGroup*> (new BusyPollGroup ()));
    return grab (static_cast <ActivePollGroup*> (new WsaPollGroup ()));
#else
//    return grab (static_cast <ActivePollGroup*> (new BusyPollGroup ()));
    return grab (static_cast <ActivePollGroup*> (new EpollPollGroup ()));
//    return grab (static_cast <ActivePollGroup*> (new SelectPollGroup ()));
#endif
}

Ref< ObjectFactory<ActivePollGroup> >
getDefaultFactory_ActivePollGroup ()
{
    class Factory : public ObjectFactory<ActivePollGroup>,
		    public virtual Object
    {
    public:
	Ref<ActivePollGroup> createNew ()
	    throw (InternalException)
	{
	    return ActivePollGroup::createDefault ();
#if 0
#ifdef PLATFORM_WIN32
	    return grab (static_cast <ActivePollGroup*> (new WsaPollGroup ()));
#else
//	    return grab (static_cast <ActivePollGroup*> (new SelectPollGroup ()));
	    return grab (static_cast <ActivePollGroup*> (new BusyPollGroup ()));
//	    return grab (static_cast <ActivePollGroup*> (new EpollPollGroup ()));
#endif
#endif
	}
    };

    return grab (static_cast < ObjectFactory<ActivePollGroup>* > (new Factory ()));
}

}

