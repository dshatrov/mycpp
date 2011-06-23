#include <mycpp/print_task.h>
#include <mycpp/util.h>

#include <mycpp/exception.h>

namespace MyCpp {

Ref<String>
Exception::getMessage () const
{
    if (!cause.isNull ()) {
	if (!message->isNullString ())
	    return String::forPrintTask ((Pr (message)) (Pr (": ")) (Pr (cause->getMessage ())));
	else
	    return cause->getMessage ();
    }

    return message;
}

Exception::Exception (String    *message,
		      Exception *cause)
{
    cloneMethod = _clone;
    raiseMethod = _raise;

    if (message == NULL)
	/* Not to be too picky about <t>String</t> usage rules,
	 * fix misuses silently. */
	this->message = String::nullString ();
    else
	this->message = message;

    this->cause = cause;

    if (cause == NULL)
	this->backtrace = collectBacktrace ();
    else
	this->backtrace = cause->getBacktrace ();
}

}

