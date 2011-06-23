#ifndef __MYCPP__NATIVE_ASYNC_FILE_H__
#define __MYCPP__NATIVE_ASYNC_FILE_H__

#include <mycpp/async_file.h>
#include <mycpp/pollable.h>
#include <mycpp/mutex.h>
#include <mycpp/rw_lock.h>

namespace MyCpp {

/*c A wrapper for native files.
 *
 * Internally the methods of this class use native
 * filesystem calls specific to an operating system
 * being used. */
class NativeAsyncFile : public AsyncFile,
			public Pollable,
			public virtual Object
{
protected:
  // mt_const

    Bool should_close;

  // (end mt_const)

  // mt_lock rw_lock

    // mt_const till ioActor_doPreClosed is called
    int fd;

  // (end mt_lock rw_lock)

  // mt_mutex state_mutex

    Bool is_error,
	 is_closed;

  // (end mt_mutex state_mutex)

    RwLock rw_lock;

    Mutex state_mutex;

    void notifyError ();

    void do_close ();

public:
  // AsyncFile interface

    IOResult read (MemoryDesc const &mem,
		   Size *nread)
	    throw (IOException,
		   InternalException);

    IOResult write (ConstMemoryDesc const &mem,
		    Size *nwritten)
	     throw (IOException,
		    InternalException);

    void seek (Int64      offset,
	       SeekOrigin origin)
	throw (IOException,
	       InternalException);

    Uint64 tell ()
	  throw (InternalException);

    void flush ()
	 throw (IOException,
		InternalException);

    bool sync (CallbackDesc<SyncCallback> const *cb,
	       Ref<SyncSubscription> *ret_sbn)
	throw (IOException,
	       InternalException);

    bool close (CallbackDesc<ClosedCallback> const *cb,
		Ref<GenericInformer::Subscription> *ret_sbn,
		bool flush_data)
	 throw (IOException,
		InternalException);

#if 0
Unnecessary
    bool isClosed_subscribe (CallbackDesc<ClosedCallback> const *cb,
			     unsigned long flags,
			     Ref<GenericInformer::Subscription> *ret_sbn);
#endif

    bool isError_subscribe (CallbackDesc<ErrorCallback> const *cb,
			    unsigned long flags,
			    Ref<GenericInformer::Subscription> *ret_sbn);

  // (End of AsyncFile interface)

  // IoActor interface

    void processInput ();

    void processOutput ();

    void processError ();

  // (end IoActor interface)

  // Pollable interface

    int getFd ();

    void lockFd ();

    void unlockFd ();

  // (end Pollable interface)

    /*m The constructor.
     *
     * @filename The name of the file to open.
     * @accessMode Access mode for the opened file. */
    NativeAsyncFile (const char    *filename,
		     unsigned long  openFlags,
		     AccessMode     accessMode)
	      throw (IOException,
		     InternalException);

    // TODO NoAutoClose
    // AutoCloseWarn
    // AutoCloseNoWarn
    NativeAsyncFile (int fd,
		     Bool should_close);

    ~NativeAsyncFile ();
};

}

#endif /* __MYCPP__NATIVE_ASYNC_FILE_H__ */

