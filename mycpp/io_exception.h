#ifndef __MYCPP__IO_EXCEPTION_H__
#define __MYCPP__IO_EXCEPTION_H__

#include <mycpp/exception.h>

namespace MyCpp {

/*c An abstract input/output exception.
 *
 * This exception is expected to be thrown
 * when an input/output exception occurs.
 * That is, the data can not be transmitted
 * or received as requested by the caller. */
class IOException : public Exception,
		    public ExceptionBase <IOException>
{
public:
    /*m Standard exception constructor. */
    IOException (String *message = String::nullString (),
		 Exception *cause = NULL)
	: Exception (message, cause)
    {
    }
};

}

#endif /* __MYCPP__IO_EXCEPTION_H__ */

