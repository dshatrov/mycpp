#ifndef __MYCPP__POLLABLE_H__
#define __MYCPP__POLLABLE_H__

#ifdef PLATFORM_WIN32
#include <Windows.h>
#endif

#include <mycpp/types.h>
#include <mycpp/mutex.h>
#include <mycpp/informer.h>
#include <mycpp/io_actor.h>

namespace MyCpp {

/*c An object which is pollable by a file descriptor.
 *
 * Pollable objects can be polled using a <t>PollGroup</t>.
 * This interface is ditinct from IoActor because this one
 * contains OS-specific methods.
 */
class Pollable : public IoActor_Closing,
		 public virtual Object
{
public:
    /*m Returns a file descriptor that should
     * be added to a <t>PollGroup</t>.
     *
     * This method is platform-specifig (for POSIX systems).
     *
     * Note that there is no race between getFd() and close() methods,
     * because file descriptors should be actually closed only
     * inside destructors of Pollable objects.
     *
     * NOTE: This method should be called only for locked fds.
     *
     * -1 can be returned (no descriptor/wrong descriptor).
     */
#ifdef PLATFORM_WIN32
    virtual int getFd () = 0;
#else
    virtual int getFd () = 0;
#endif

    // TODO It looks like these methods are unused and can safely be removed.
    virtual void lockFd () = 0;

    virtual void unlockFd () = 0;
};

}

#endif /* __MYCPP__POLLABLE_H__ */

