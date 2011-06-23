#ifndef __MYCPP__WAIT_GROUP_H__
#define __MYCPP__WAIT_GROUP_H__

#include <mycpp/list.h>
#include <mycpp/waitable.h>
#include <mycpp/data_mutex.h>
#include <mycpp/cond.h>

namespace MyCpp {

// NOTE 08.07.07: This class is unused to date, I only keep it around
// because the implementation looks sane. It's not tested though,
// and thus it's not guaranteed to be suitable for any purpose.

class WaitGroup : public virtual Object
{
public:
    class WaitableRecord : public SimplyReferenced
    {
	friend class WaitGroup;

    protected:
	enum State {
	    UnknownState,
	    ActiveState,
	    InactiveState
	};

	bool valid;
	WaitGroup *waitGroup;

	WeakRef<Waitable> weak_waitable;
	Ref< Informer<Waitable::ActiveCallback>::Subscription > activeSbn;

	State state;

	List< Ref<WaitableRecord> >::Element *wgLink;

	void *userData;
	Ref<SimplyReferenced> ref_data;

    public:
	void* getUserData ()
	{
	    return userData;
	}
    };

protected:
    List< Ref<WaitableRecord> > activeList;
    List< Ref<WaitableRecord> > inactiveList;

    DataMutex activeMutex;
    Cond activeCond;

    static void activeEvent (void *_wr);

public:
    Ref<WaitableRecord> wait ();

    Ref<WaitableRecord> add (Waitable         *waitable,
			     void             *user_data,
			     SimplyReferenced *ref_data);

    void remove (WaitableRecord *wr);

    WaitGroup ();
};

}

#endif /* __MYCPP__WAIT_GROUP_H__ */

