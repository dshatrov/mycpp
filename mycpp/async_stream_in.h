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

#ifndef __MYCPP__ASYNC_STREAM_IN_H__
#define __MYCPP__ASYNC_STREAM_IN_H__

#include <mycpp/types.h>
#include <mycpp/object.h>
#include <mycpp/informer.h>
#include <mycpp/io_exception.h>
#include <mycpp/internal_exception.h>

namespace MyCpp {

class AsyncStreamIn : public virtual Object
{
public:
    typedef void (*InputCallback) (void *data);

    typedef void (*ErrorCallback) (void *data);

    typedef void (*ShutdownCallback) (Exception *exc,
				      void *data);

protected:
    Ref< Informer<InputCallback> > input_informer;
    Ref< Informer<ErrorCallback> > error_informer;
    Ref< Informer<ShutdownCallback> > shutdown_informer;

    static void inputInform (InputCallback callback,
			     void *callback_data,
			     void *data);

    static void errorInform (ErrorCallback callback,
			     void *callback_data,
			     void *data);

    static void shutdownInform (ShutdownCallback callback,
				void *callback_data,
				void *data);

    void fireInput ()
    {
	input_informer->informAll (NULL);
    }

    void fireError ()
    {
	error_informer->informAll (NULL);
    }

    class ShutdownData : public virtual SimplyReferenced
    {
    public:
	Exception *exc;
    };

    void fireShutdown (Exception *exc)
    {
	ShutdownData data;
	data.exc = exc;
	shutdown_informer->informAll (&data);
    }

    Ref< Informer<ShutdownCallback> > getShutdownInformer ()
    {
	return shutdown_informer;
    }

public:
    virtual IOResult read (MemoryDesc const &mem,
			   Size *nread)
		    throw (IOException,
			   InternalException) = 0;

    // Analogous to 'AsyncFile.setNeedInput (true)'
    virtual void requestInput () = 0;

    // TODO Introduce ShutdownSubscription instead of GenericInformer::Subscription.
    // Current interface forces spurious notifications about weaker shutdown events
    // for those who expect stronger ones.
    virtual bool shutdown (CallbackDesc<ShutdownCallback> const *cb,
			   Ref<GenericInformer::Subscription> *ret_sbn)
		    throw (IOException,
			   InternalException) = 0;

    virtual bool isError_subscribe (const CallbackDesc<ErrorCallback> *cb,
				    unsigned long flags,
				    Ref<GenericInformer::Subscription> *ret_sbn) = 0;

    Ref< Informer<InputCallback> > getInputInformer ()
    {
	return input_informer;
    }

    Ref< Informer<ErrorCallback> > getErrorInformer ()
    {
	return error_informer;
    }

    AsyncStreamIn ();

    // See FileAsyncStreamIn() for ideas on why this constructor is here.
    AsyncStreamIn (Informer<InputCallback> *input_informer,
		   Informer<ErrorCallback> *error_informer);
};

}

#endif /* __MYCPP__ASYNC_STREAM_IN_H__ */

