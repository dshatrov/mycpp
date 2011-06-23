#include <mycpp/immediate_trigger.h>

namespace MyCpp {

void
ImmediateTrigger::trigger ()
{
    stateMutex.lock ();

    if (inTrigger) {
	needTrigger = true;
	stateMutex.unlock ();
	return;
    }

    inTrigger = true;
    do {
	needTrigger = false;
	stateMutex.unlock ();

	fireEvent ();

	stateMutex.lock ();
    } while (needTrigger);
    inTrigger = false;

    stateMutex.unlock ();
}

ImmediateTrigger::ImmediateTrigger ()
{
    inTrigger = false;
    needTrigger = false;
}

}

