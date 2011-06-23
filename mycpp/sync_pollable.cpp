#include <mycpp/sync_pollable.h>
#include <mycpp/util.h>

namespace MyCpp {

void
SyncPollable::doInput ()
{
    processingInput = true;
    inputDirty = false;
    stateMutex.unlock ();

    for (;;) {
	pollable->processInput ();

	stateMutex.lock ();

	if (processingError) {
	    if (errorDirty) {
		processingInput = false;
		return;
	    }

	    /* If we are processing an error,
	     * then output and input processing threads
	     * have already collapsed into one thread,
	     * and we have to call output handlers
	     * from within input handlers, and vice versa. */

	    if (processingOutput == false)
		if (outputDirty)
		    doOutput ();
	} else
	    if (errorDirty)
		doError ();

	if (inputDirty == false) {
	    processingInput = false;
	    return;
	}

	inputDirty = false;
	stateMutex.unlock ();
    }
}

void
SyncPollable::doOutput ()
{
    processingOutput = true;
    outputDirty = false;
    stateMutex.unlock ();

    for (;;) {
	pollable->processOutput ();

	stateMutex.lock ();

	if (processingError) {
	    if (errorDirty) {
		processingOutput = false;
		return;
	    }

	    /* If we are processing an error,
	     * then output and input processing threads
	     * have already collapsed into one thread,
	     * and we have to call output handlers
	     * from within input handlers, and vice versa. */

	    if (processingInput == false)
		if (inputDirty)
		    doInput ();
	} else
	    if (errorDirty)
		doError ();

	if (outputDirty == false) {
	    processingOutput = false;
	    return;
	}

	outputDirty = false;
	stateMutex.unlock ();
    }
}

void
SyncPollable::doError ()
{
    processingError = true;
    errorDirty = false;
    stateMutex.unlock ();

    for (;;) {
	pollable->processError ();

	stateMutex.lock ();
	if (errorDirty == false) {
	    if (inputDirty &&
		processingInput == false)
	    {
		doInput ();
	    }

	    if (outputDirty &&
		processingOutput == false)
	    {
		doOutput ();
	    }

	    if (errorDirty == false) {
		processingError = false;
		return;
	    }
	}

	errorDirty = false;
	stateMutex.unlock ();
    }
}

int
SyncPollable::getFd ()
{
    return pollable->getFd ();
}

void
SyncPollable::processInput ()
{
    stateMutex.lock ();

    if (processingInput ||
	processingError)
    {
	inputDirty = true;
    } else
	doInput ();

    stateMutex.unlock ();
}

void SyncPollable::processOutput ()
{
    stateMutex.lock ();

    if (processingOutput ||
	processingError)
    {
	outputDirty = true;
    } else
	doOutput ();

    stateMutex.unlock ();
}

void
SyncPollable::processError ()
{
    stateMutex.lock ();

    if (processingInput  ||
	processingOutput ||
	processingError)
    {
	errorDirty = true;
    } else
	doError ();

    stateMutex.unlock ();
}

SyncPollable::SyncPollable (Pollable *pollable)
{
    if (pollable == NULL)
	abortIfReached ();

    this->pollable = pollable;

    inputDirty  = false;
    outputDirty = false;
    errorDirty  = false;

    processingInput  = false;
    processingOutput = false;
    processingError  = false;
}

}

