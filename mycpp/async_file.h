/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2009-2010 Dmitry Shatrov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __MYCPP__ASYNC_FILE_H__
#define __MYCPP__ASYNC_FILE_H__

#include <mycpp/types.h>
#include <mycpp/object.h>
#include <mycpp/informer.h>
#include <mycpp/io_subject.h>
#include <mycpp/io_exception.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

/*c Asynchronous file abstraction. */
class AsyncFile : public virtual IoSubject,
		  public virtual Object
{
public:
    /*t Input callback.
     *
     * @data User data. */
    typedef void (*InputCallback) (void *data);

    typedef void (*OutputCallback) (void *data);

    typedef void (*ErrorCallback) (void *data);

    typedef void (*ClosedCallback) (Exception *exc,
				    void      *data);

protected:
    Ref< Informer<InputCallback> >       input_informer;
    Ref< Informer<OutputCallback> >      output_informer;
    Ref< Informer<ErrorCallback> >       error_informer;
    Ref< Informer<ClosedCallback> >      closed_informer;

    static void inputInform (InputCallback callback,
			     void *callback_data,
			     void *data);

    static void outputInform (OutputCallback callback,
			      void *callback_data,
			      void *data);

    static void errorInform (ErrorCallback callback,
			     void *callback_data,
			     void *data);

    static void closedInform (ClosedCallback callback,
			      void *callback_data,
			      void *data);

    void fireInput ()
    {
	input_informer->informAll (NULL);
    }

    void fireOutput ()
    {
	output_informer->informAll (NULL);
    }

    void fireError ()
    {
	error_informer->informAll (NULL);
    }

    class ClosedData : public virtual SimplyReferenced
    {
    public:
	Exception *exc;
    };

    void fireClosed (Exception *exc)
    {
	ClosedData data;
	data.exc = exc;
	closed_informer->informAll (&data);
    }

public:
    virtual IOResult read (MemoryDesc const &mem,
			   Size *nread)
		    throw (IOException,
			   InternalException) = 0;

    virtual IOResult write (ConstMemoryDesc const &mem,
			    Size *nwritten)
		     throw (IOException,
			    InternalException) = 0;

    virtual void seek (Int64      offset,
		       SeekOrigin origin)
		throw (IOException,
		       InternalException) = 0;

    virtual Uint64 tell ()
		  throw (InternalException) = 0;

    virtual void flush ()
		 throw (IOException,
			InternalException) = 0;

    class SyncSubscription : public virtual SimplyReferenced {};

    typedef void (*SyncCallback) (Exception *exc,
				  void       *data);

    virtual bool sync (CallbackDesc<SyncCallback> const *cb,
		       Ref<SyncSubscription> *ret_sbn)
		throw (IOException,
		       InternalException) = 0;

    virtual bool close (CallbackDesc<ClosedCallback> const *cb,
			Ref<GenericInformer::Subscription> *ret_sbn,
			bool flush_data)
		 throw (IOException,
			InternalException) = 0;

#if 0
This is unnecessary and creates confusion.
    virtual bool isClosed_subscribe (CallbackDesc<ClosedCallback> const *cb,
				     unsigned long flags,
				     Ref<GenericInformer::Subscription> *ret_sbn) = 0;
#endif

    virtual bool isError_subscribe (CallbackDesc<ErrorCallback> const *cb,
				    unsigned long flags,
				    Ref<GenericInformer::Subscription> *ret_sbn) = 0;

    Ref< Informer<InputCallback> > getInputInformer ()
    {
	return input_informer;
    }

    Ref< Informer<OutputCallback> > getOutputInformer ()
    {
	return output_informer;
    }

    Ref< Informer<ErrorCallback> > getErrorInformer ()
    {
	return error_informer;
    }

#if 0
This is really unnecessary and creates confusion.
    // NOTE: There isn't any particular need in getClosedInformer(),
    // but it is here because an informer is used to simplify notifications
    // on completion of deferred invocations of close(). As long as we
    // have closed_informer, there's no reason not to provide getClosedInformer().
    Ref< Informer<ClosedCallback> > getClosedInformer ()
    {
	return closed_informer;
    }
#endif

    AsyncFile ();
};

}

#endif /* __MYCPP__ASYNC_FILE_H__ */

