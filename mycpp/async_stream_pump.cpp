#include <mycpp/util.h>
#include <mycpp/io.h>

#include <mycpp/async_stream_pump.h>

#define DEBUG(a) ;

namespace MyCpp {

void
AsyncStreamPump::errorInform (ErrorCallback callback,
			      void *callback_data,
			      void * /* data */)
{
    callback (callback_data);
}

void
AsyncStreamPump::closedInform (ClosedCallback callback,
			       void *callback_data,
			       void * /* data */)
{
    callback (callback_data);
}

// mt_sync transmit_trigger
void
AsyncStreamPump::do_transmit ()
{
    DEBUG (
	errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit").pendl ();
    )

    Size data_len = in_buffer;

    bool got_eof_in;

    state_mutex.lock ();

    if (is_error || is_closed) {
	state_mutex.unlock ();
	DEBUG (
	    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: is_error || is_closed").pendl ();
	)
	return;
    }

    if (data_len == 0 && eof_in) {
	state_mutex.unlock ();
	DEBUG (
	    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: data_len == 0 && eof_in").pendl ();
	)
	// FIXME Correct?
	return;
    }

    got_eof_in = eof_in;

    state_mutex.unlock ();

    bool got_again_in = false;
    bool got_again_out = false;

    if (data_len == 0) {
	abortIf (got_eof_in);

	try {
	    while (data_len < buffer_size) {
		Size nread;
//#if 0
		DEBUG (
		    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: calling read()").pendl ();
		)
//#endif
		IOResult res = stream_in->read (MemoryDesc (buffer, buffer_size).getRegionOffset (data_len), &nread);
//#if 0
		DEBUG (
		    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: read() returned").pendl ();
		)
//#endif
		if (res == IOResultAgain) {
		    DEBUG (
			errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: read: IOResultAgain").pendl ();
		    )
		    got_again_in = true;
		    break;
		} else
		if (res == IOResultEof) {
		    DEBUG (
			errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: read: IOResultEof").pendl ();
		    )
		    got_eof_in = true;
		    break;
		}

		abortIf (res != IOResultNormal);
		abortIf (nread > buffer_size - data_len);

		DEBUG (
		    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: ").print (nread).print (" bytes read").pendl ();
		)
		data_len += nread;
	    }
	} catch (Exception &exc) {
	    DEBUG (
		errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: read: exception").pendl ();
	    )
	    state_mutex.lock ();
	    is_error = true;
	    state_mutex.unlock ();

	    fireError ();
	    return;
	}
    }

    abortIf (data_len > buffer_size);
    in_buffer = data_len;

    try {
	while (cur_pos < data_len) {
	    Size nwritten;
//#if 0
	    DEBUG (
		errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: calling write()").pendl ();
	    )
//#endif
	    IOResult res = stream_out->write (ConstMemoryDesc (buffer, data_len).getRegionOffset (cur_pos), &nwritten);
//#if 0
	    DEBUG (
		errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: write() returned").pendl ();
	    )
//#endif
	    if (res == IOResultAgain) {
		DEBUG (
		    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: write: IOResultAgain").pendl ();
		)
		got_again_out = true;
		break;
	    } else
	    if (res == IOResultEof) {
		DEBUG (
		    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: write: IOResultEof").pendl ();
		)
		state_mutex.lock ();
		is_error = true;
		state_mutex.unlock ();

		fireError ();
		return;
	    }

	    abortIf (res != IOResultNormal);
	    abortIf (nwritten > data_len - cur_pos);

	    DEBUG (
		errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: ").print (nwritten).print (" bytes written").pendl ();
	    )
	    cur_pos += nwritten;

	    if (nwritten > 0)
		flush_needed = true;

	    // TODO TEMPORAL, should flush only when necessary
	    stream_out->flush ();
	}
    } catch (Exception &exc) {
	DEBUG (
	    errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: write: exception").pendl ();
	)

	state_mutex.lock ();
	is_error = true;
	state_mutex.unlock ();

	fireError ();
	return;
    }

    abortIf (cur_pos > data_len);
    if (cur_pos == data_len) {
	if (flush_needed && (got_again_in || data_len == buffer_size)) {
	    flush_needed = false;
	    try {
		stream_out->flush ();
	    } catch (Exception &exc) {
		state_mutex.lock ();
		is_error = true;
		state_mutex.unlock ();

		fireError ();
		return;
	    }
	}

	in_buffer = 0;
	cur_pos = 0;

	if (got_eof_in) {
	    DEBUG (
		errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: got_eof_in").pendl ();
	    )

	    state_mutex.lock ();
	    eof_in = true;
	    is_closed = true;
	    state_mutex.unlock ();

	    fireClosed ();
	    return;
	}

	if (!got_again_in)
	    stream_in->requestInput ();
    } else {
	if (!got_again_out)
	    stream_out->requestOutput ();
    }

//#if 0
    DEBUG (
	errf->print ((UintPtr) this).print (" MyCpp.AsyncStreamPump.do_transmit: done").pendl ();
    )
//#endif
}

void
AsyncStreamPump::transmit_trigger_callback (void *_self)
try {
    AsyncStreamPump * const &self = static_cast <AsyncStreamPump*> (_self);
    self->do_transmit ();
} catch (Exception &exc) {
    abortIfReached ();
}

void
AsyncStreamPump::stream_in__input_callback (void *_self)
try {
    AsyncStreamPump * const &self = static_cast <AsyncStreamPump*> (_self);

    DEBUG (
	errf->print ((UintPtr) self).print (" MyCpp.AsyncStreamPump.stream_in__input_callback").pendl ();
    )

    self->transmit_trigger.trigger ();
} catch (Exception &exc) {
    abortIfReached ();
}

void
AsyncStreamPump::stream_out__output_callback (void *_self)
try {
    AsyncStreamPump * const &self = static_cast <AsyncStreamPump*> (_self);

    DEBUG (
	errf->print ((UintPtr) self).print (" MyCpp.AsyncStreamPump.stream_out__output_callback").pendl ();
    )

    self->transmit_trigger.trigger ();
} catch (Exception &exc) {
    abortIfReached ();
}

void
AsyncStreamPump::stream_in__error_callback (void *_self)
try {
    AsyncStreamPump * const &self = static_cast <AsyncStreamPump*> (_self);

    DEBUG (
	errf->print ((UintPtr) self).print (" MyCpp.AsyncStreamPump.stream_in__error_callback").pendl ();
    )

    self->state_mutex.lock ();
    if (self->is_error) {
	self->state_mutex.unlock ();
	return;
    }

    self->is_error = true;
    self->state_mutex.unlock ();

    self->fireError ();
} catch (Exception &exc) {
    abortIfReached ();
}

void
AsyncStreamPump::stream_out__error_callback (void *_self)
try {
    AsyncStreamPump * const &self = static_cast <AsyncStreamPump*> (_self);

    DEBUG (
	errf->print ((UintPtr) self).print (" MyCpp.AsyncStreamPump.stream_out__error_callback").pendl ();
    )

    self->state_mutex.lock ();
    if (self->is_error) {
	self->state_mutex.unlock ();
	return;
    }

    self->is_error = true;
    self->state_mutex.unlock ();

    self->fireError ();
} catch (Exception &exc) {
    abortIfReached ();
}

bool
AsyncStreamPump::isError_subscribe (CallbackDesc<ErrorCallback> const *cb,
				    Ref<GenericInformer::Subscription> *ret_sbn)
{
    if (ret_sbn != NULL)
	*ret_sbn = NULL;

    state_mutex.lock ();
    if (!is_error) {
	if (cb != NULL) {
	    Ref<GenericInformer::Subscription> sbn = getErrorInformer ()->subscribe (*cb, GenericInformer::InformOneshot);
	    if (ret_sbn != NULL)
		*ret_sbn = sbn;
	}

	state_mutex.unlock ();

	return false;
    }
    state_mutex.unlock ();

    return true;
}

bool
AsyncStreamPump::isClosed_subscribe (CallbackDesc<ClosedCallback> const *cb,
				     Ref<GenericInformer::Subscription> *ret_sbn)
{
    if (ret_sbn != NULL)
	*ret_sbn = NULL;

    state_mutex.lock ();
    if (!is_closed) {
	if (cb != NULL) {
	    Ref<GenericInformer::Subscription> sbn = getClosedInformer ()->subscribe (*cb, GenericInformer::InformOneshot);
	    if (ret_sbn != NULL)
		*ret_sbn = sbn;
	}

	state_mutex.unlock ();

	return false;
    }
    state_mutex.unlock ();

    return true;
}

AsyncStreamPump::AsyncStreamPump (AsyncStreamIn *stream_in,
				  AsyncStreamOut *stream_out,
				  Size buffer_size)
{
    abortIf (stream_in   == NULL ||
	     stream_out  == NULL ||
	     buffer_size == 0);

    DEBUG (
	errf->print ((UintPtr) this).print ("MyCpp.AsyncStreamPump.()").pendl ();
    )

    error_informer = grab (new Informer<ErrorCallback> (errorInform));
    closed_informer = grab (new Informer<ClosedCallback> (closedInform));

    this->stream_in = stream_in;
    this->stream_out = stream_out;
    this->buffer_size = buffer_size;

    buffer.allocate (buffer_size);
    cur_pos = 0;
    in_buffer = 0;

  // mt_start

    state_mutex.lock ();

    {
	CallbackDesc<Trigger::EventCallback> cb;
	cb.weak_obj = this;
	cb.callback = transmit_trigger_callback;
	cb.callbackData = this;

	transmit_trigger.getEventInformer ()->subscribe (cb);
    }

    {
	CallbackDesc<AsyncStreamIn::InputCallback> cb;
	cb.weak_obj = this;
	cb.callback = stream_in__input_callback;
	cb.callbackData = this;

	stream_in->getInputInformer ()->subscribe (cb);
    }

    {
	CallbackDesc<AsyncStreamIn::ErrorCallback> cb;
	cb.weak_obj = this;
	cb.callback = stream_in__error_callback;
	cb.callbackData = this;

	if (stream_in->isError_subscribe (&cb, GenericInformer::InformOneshot, NULL /* ret_sbn */))
	    is_error = true;
    }

    {
	CallbackDesc<AsyncStreamOut::OutputCallback> cb;
	cb.weak_obj = this;
	cb.callback = stream_out__output_callback;
	cb.callbackData = this;

	stream_out->getOutputInformer ()->subscribe (cb);
    }

    {
	CallbackDesc<AsyncStreamOut::ErrorCallback> cb;
	cb.weak_obj = this;
	cb.callback = stream_out__error_callback;
	cb.callbackData = this;

	if (stream_out->isError_subscribe (&cb, GenericInformer::InformOneshot, NULL /* ret_sbn */))
	    is_error = true;
    }

    state_mutex.unlock ();

    stream_in->requestInput ();

    // TEST
//    stream_out->requestOutput ();
}

}

