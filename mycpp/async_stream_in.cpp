#include <mycpp/async_stream_in.h>

namespace MyCpp {

void
AsyncStreamIn::inputInform (InputCallback callback,
			    void *callback_data,
			    void * /* data */)
{
    callback (callback_data);
}

void
AsyncStreamIn::errorInform (ErrorCallback callback,
			    void *callback_data,
			    void * /* data */)
{
    callback (callback_data);
}

void
AsyncStreamIn::shutdownInform (ShutdownCallback callback,
			       void *callback_data,
			       void *_data)
{
    ShutdownData * const &data = static_cast <ShutdownData*> (_data);
    callback (data->exc, callback_data);
}

AsyncStreamIn::AsyncStreamIn ()
{
    input_informer = grab (new Informer<InputCallback> (inputInform));
    error_informer = grab (new Informer<ErrorCallback> (errorInform));
    shutdown_informer = grab (new Informer<ShutdownCallback> (shutdownInform));
}

AsyncStreamIn::AsyncStreamIn (Informer<InputCallback> *input_informer,
			      Informer<ErrorCallback> *error_informer)
{
    if (input_informer != NULL)
	this->input_informer = input_informer;
    else
	this->input_informer = grab (new Informer<InputCallback> (inputInform));

    if (error_informer != NULL)
	this->error_informer = error_informer;
    else
	this->error_informer = grab (new Informer<ErrorCallback> (errorInform));

    shutdown_informer = grab (new Informer<ShutdownCallback> (shutdownInform));
}

}

