#include <mycpp/async_stream_out.h>

namespace MyCpp {

void
AsyncStreamOut::outputInform (OutputCallback callback,
			      void *callback_data,
			      void * /* data */)
{
    callback (callback_data);
}

void
AsyncStreamOut::errorInform (ErrorCallback callback,
			     void *callback_data,
			     void * /* data */)
{
    callback (callback_data);
}

void
AsyncStreamOut::shutdownInform (ShutdownCallback callback,
				void *callback_data,
				void *_data)
{
    ShutdownData * const &data = static_cast <ShutdownData*> (_data);
    callback (data->exc, callback_data);
}

AsyncStreamOut::AsyncStreamOut ()
{
    output_informer   = grab (new Informer<OutputCallback>   (outputInform));
    error_informer    = grab (new Informer<ErrorCallback>    (errorInform));
    shutdown_informer = grab (new Informer<ShutdownCallback> (shutdownInform));
}

}

