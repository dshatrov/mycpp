#include <cstdlib>
#include <unistd.h>
#include <errno.h>

#include <mycpp/mycpp.h>
#include <mycpp/util.h>
#include <mycpp/io.h>
#include <mycpp/object.h>
#include <mycpp/mutex.h>
#include <mycpp/thread.h>
#include <mycpp/epoll_poll_group.h>
#include <mycpp/native_async_file.h>

using namespace MyCpp;

class Test : public virtual Object
{
protected:
  // mt_mutex state_mutex

    Bool should_stop;

  // mt_mutex state_mutex

    Mutex state_mutex;

public:
    bool shouldStop ()
    {
	bool tmp_stop;

	state_mutex.lock ();
	tmp_stop = should_stop;
	state_mutex.unlock ();

	return tmp_stop;
    }

    void stop ()
    {
	state_mutex.lock ();
	should_stop = true;
	state_mutex.unlock ();
    }
};

// mt_const

  Ref<ActivePollGroup> poll_group;

// (end mt_const)

static void
thread_one_proc (void *_test)
{
    Test * const &test = static_cast <Test*> (_test);

    while (!test->shouldStop ()) {
	poll_group->iteration ();
	errf->print ("thread_one_proc: iteration complete").pendl ();
    }

    errf->print ("thread_one_proc: done").pendl ();
}

static void
thread_two_proc (void *_test)
{
    Test * const &test = static_cast <Test*> (_test);

    while (!test->shouldStop ()) {
	poll_group->iteration ();
	errf->print ("thread_two_proc: iteration complete").pendl ();
    }

    errf->print ("thread_two_proc: done").pendl ();
}

int main (void)
try {
    myCppInit ();

    int rv;

    int p [2];
    rv = pipe (p);
    if (rv == -1) {
	throw IOException (errnoToString (errno));
    } else
    if (rv != 0) {
	errf->print ("main: WARNING: unexpected return value from pipe()").pendl ();
	throw IOException ();
    }

    Ref<NativeAsyncFile> pipe_in  = grab (new NativeAsyncFile (p [0], true /* should_close */)),
			 pipe_out = grab (new NativeAsyncFile (p [1], true /* should_close */));

    poll_group = grab (static_cast <PollGroup*> (new EpollPollGroup));

    Ref<Test> test = grab (new Test);

  // mt_const barrier

    Ref<Thread> thread_one = grab (new Thread (thread_one_proc,
					       test,   // thread_func_data
					       NULL,   // ref_data
					       true)); // joinable

    Ref<Thread> thread_two = grab (new Thread (thread_two_proc,
					       test,   // thread_func_data
					       NULL,   // ref_data
					       true)); // joinable

    for (int i = 0; i < 3; i++) {
	uSleep (1000000);
	errf->print (".").pflush ();
    }
    errf->pendl ();

    test->stop ();
    poll_group->trigger ();

    thread_one->join ();
    thread_two->join ();

    errf->print ("main: done").pendl ();

    errf->print ("SUCCESS").pendl ();
    return 0;
} catch (Exception &exc) {
    errf->print ("main: exception: ").print (exc.getMessage ()).pendl ();
    errf->print ("FAILURE").pendl ();
    return EXIT_FAILURE;
}

