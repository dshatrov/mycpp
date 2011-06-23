#include <mycpp/async_file.h>

namespace MyCpp {

void
AsyncFile::inputInform (InputCallback callback,
			void *callback_data,
			void * /* data */)
{
    callback (callback_data);
}

void
AsyncFile::outputInform (OutputCallback callback,
			 void *callback_data,
			 void * /* data */)
{
    callback (callback_data);
}

void
AsyncFile::errorInform (ErrorCallback callback,
			void *callback_data,
			void * /* data */)
{
    callback (callback_data);
}

void
AsyncFile::closedInform (ClosedCallback callback,
			 void *callback_data,
			 void *_data)
{
    ClosedData * const &data = static_cast <ClosedData*> (_data);
    callback (data->exc, callback_data);
}

AsyncFile::AsyncFile ()
{
    input_informer  = grab (new Informer<InputCallback>  (inputInform));
    output_informer = grab (new Informer<OutputCallback> (outputInform));
    error_informer  = grab (new Informer<ErrorCallback>  (errorInform));
    closed_informer = grab (new Informer<ClosedCallback> (closedInform));
}

}

