#ifndef __MYCPP__POLLER_H__
#define __MYCPP__POLLER_H__

#include <mycpp/object.h>
#include <mycpp/poll_group.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

class Poller : public PollGroup,
	       public virtual Object
{
public:
    /*m Runs the poll and blocks. run() can be called only once for each poller.
     */
    virtual void run ()
	       throw (InternalException) = 0;

    /*m Order all spawned threads to be stopped.
     */
    virtual void stopNoJoin ()
		      throw (InternalException) = 0;

    /*m Wait for all spawned threads to be joined.
     */
    virtual void join ()
		throw (InternalException) = 0;

    /*m Creates an objects representing default implementation
     * of Poller interface for the current architecture.
     */
    static Ref<Poller> createDefault ()
			       throw (InternalException);
};

}

#endif /* __MYCPP__POLLER_H__ */

