#include <mycpp/file_async_stream_out.h>
#include <mycpp/util.h>

namespace MyCpp {

void
FileAsyncStreamOut::file_output_callback (void *_self)
{
    FileAsyncStreamOut * const &self = static_cast <FileAsyncStreamOut*> (_self);

    self->fireOutput ();
}

void
FileAsyncStreamOut::file_error_callback (void *_self)
{
    FileAsyncStreamOut * const &self = static_cast <FileAsyncStreamOut*> (_self);

    self->state_mutex.lock ();
    self->is_error = true;
    self->state_mutex.unlock ();

    self->fireError ();
}

IOResult
FileAsyncStreamOut::write (ConstMemoryDesc const &mem,
			   Size *nwritten)
    throw (IOException,
	   InternalException)
{
    return file->write (mem.getMemory (), mem.getLength (), nwritten);
}

void
FileAsyncStreamOut::requestOutput ()
{
    file->setNeedOutput (true);
}

void
FileAsyncStreamOut::flush ()
    throw (IOException,
	   InternalException)
{
    file->flush ();
}

void
FileAsyncStreamOut::shutdown ()
{    
    /* No-op */
}

bool
FileAsyncStreamOut::isError ()
{
    bool ret;

    state_mutex.lock ();
    ret = this->is_error;
    state_mutex.unlock ();

    return ret;
}

bool
FileAsyncStreamOut::isError_subscribe (CallbackDesc<ErrorCallback> const &cb,
				       unsigned long flags,
				       Ref<GenericInformer::Subscription> *ret_sbn)
{
    if (ret_sbn != NULL)
	*ret_sbn = NULL;

    {
      DataMutexLock state_lock (state_mutex);

	if (is_error)
	    return true;

	if (ret_sbn != NULL)
	    *ret_sbn = getErrorInformer ()->subscribe (cb, flags);
	else
	    getErrorInformer ()->subscribe (cb, flags);
    }

    return false;
}

FileAsyncStreamOut::FileAsyncStreamOut (AsyncFile *file)
{
    abortIf (file == NULL);

    this->file = file;
}

}

