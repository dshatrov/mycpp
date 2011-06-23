#include <mycpp/file_async_stream_in.h>
#include <mycpp/util.h>

namespace MyCpp {

IOResult
FileAsyncStreamIn::read (unsigned char *buf,
			 unsigned long  len,
			 unsigned long *nread)
    throw (IOException,
	   InternalException)
{
    return async_file->read (buf, len, nread);
}

void
FileAsyncStreamIn::requestInput ()
{
    async_file->setNeedInput (true);
}

void
FileAsyncStreamIn::shutdown ()
{
    /* No-op */
}

bool
FileAsyncStreamIn::isError ()
{
    return false;
}

bool
FileAsyncStreamIn::isError_subscribe (const CallbackDesc<ErrorCallback> &cb,
				      unsigned long flags,
				      Ref<GenericInformer::Subscription> *ret_sbn)
{
    if (ret_sbn != NULL)
	*ret_sbn = NULL;

  /* Subscribing does not make any sense, since the event will never be signaled.
    if (cb != NULL) {
	Ref<GenericInformer::Subscription> sbn = getClosedInformer ()->subscribe (obj, *cb, flags);
	if (ret_sbn != NULL)
	    *ret_sbn = sbn;
    }
  */

    return false;
}

FileAsyncStreamIn::FileAsyncStreamIn (AsyncFile *async_file)
    : AsyncStreamIn (async_file->getInputInformer (), NULL)
{
    if (async_file == NULL)
	abortIfReached ();

    this->async_file = async_file;
}

}

