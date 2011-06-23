#ifndef __MYCPP__SYNCHRONIZATION_TEST_H__
#define __MYCPP__SYNCHRONIZATION_TEST_H__

#include <mycpp/types.h>
#include <mycpp/mutex.h>
#include <mycpp/util.h>

namespace MyCpp {

// Usage example:
//
//     static SynchronizationTest sync_test__static;
//     SynchronizationTest::Test sync_test (sync_test__static);
//

class SynchronizationTest
{
protected:
  // mt_mutex state_mutex

    Bool entered;

  // (end mt_mutex state_mutex)

    Mutex state_mutex;

    void enter ()
    {
	state_mutex.lock ();
	abortIf (entered);
	entered = true;
	state_mutex.unlock ();
    }

    void leave ()
    {
	state_mutex.lock ();
	abortIf (!entered);
	entered = false;
	state_mutex.unlock ();
    }

public:
    class Test
    {
	friend class SynchronizationTest;

    protected:
	SynchronizationTest &st;

    public:
	Test (SynchronizationTest &st)
	    : st (st)
	{
	    st.enter ();
	}

	~Test ()
	{
	    st.leave ();
	}
    };
};

}

#endif /* __MYCPP__SYNCHRONIZATION_TEST_H__ */

