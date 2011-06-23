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

#ifndef __MYCPP__ASYNC_STREAM_PUMP_H__
#define __MYCPP__ASYNC_STREAM_PUMP_H__

#include <mycpp/object.h>
#include <mycpp/array_holder.h>
#include <mycpp/data_mutex.h>
#include <mycpp/informer.h>
#include <mycpp/immediate_trigger.h>
#include <mycpp/async_stream_in.h>
#include <mycpp/async_stream_out.h>

namespace MyCpp {

class AsyncStreamPump : public virtual Object
{
public:
    typedef void (*ErrorCallback) (void *data);

    typedef void (*ClosedCallback) (void *data);

protected:
  // mt_const

    Ref< Informer<ErrorCallback> > error_informer;
    Ref< Informer<ClosedCallback> > closed_informer;

    Ref<AsyncStreamIn> stream_in;
    Ref<AsyncStreamOut> stream_out;

    Size buffer_size;

  // (end mt_const)

  // mt_sync transmit_trigger

    ArrayHolder<Byte> buffer;

    Bool flush_needed;

    Size cur_pos;
    Size in_buffer;

  // (end mt_sync transmit_trigger)

  // mt_mutex state_mutex

    Bool eof_in;

    Bool is_error;
    Bool is_closed;

  // (end mt_mutex state_mutex)

    ImmediateTrigger transmit_trigger;

    DataMutex state_mutex;

    static void errorInform (ErrorCallback callback,
			     void *callback_data,
			     void *data);

    static void closedInform (ClosedCallback callback,
			      void *callback_data,
			      void *data);

    void do_transmit ();

    static void transmit_trigger_callback (void *_self);

    static void stream_in__input_callback (void *_self);

    static void stream_in__error_callback (void *_self);

    static void stream_out__output_callback (void *_self);

    static void stream_out__error_callback (void *_self);

    void fireError ()
    {
	error_informer->informAll (NULL);
    }

    void fireClosed ()
    {
	closed_informer->informAll (NULL);
    }

public:
    bool isError_subscribe (CallbackDesc<ErrorCallback> const *cb,
			    Ref<GenericInformer::Subscription> *ret_sbn);

    bool isClosed_subscribe (CallbackDesc<ClosedCallback> const *cb,
			     Ref<GenericInformer::Subscription> *ret_sbn);

    Ref< Informer<ErrorCallback> > getErrorInformer ()
    {
	return error_informer;
    }

    Ref< Informer<ClosedCallback> > getClosedInformer ()
    {
	return closed_informer;
    }

    AsyncStreamPump (AsyncStreamIn *stream_in,
		     AsyncStreamOut *stream_out,
		     Size buffer_size);
};

}

#endif /* __MYCPP__ASYNC_STREAM_PUMP_H__ */

