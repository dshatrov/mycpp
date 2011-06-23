/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2008  Dmitry M. Shatrov
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

#ifndef __MYCPP__EXCEPTION_H__
#define __MYCPP__EXCEPTION_H__

#include <mycpp/simply_referenced.h>
#include <mycpp/string.h>
#include <mycpp/base_util.h>

namespace MyCpp {

class Exception;

// MyCpp exceptions are both referenced and cloneable. Cloneability means that
// we need a way to clone each of the base classes of a given exception class.
// The most straightforward way to acheive that is by using copy constructors,
// which are generated automatically by the compiler in most cases. This
// means that SimplyReferenced objects must have a valid copy constructor.

// FIXME This is wrong.
template <class T>
class ExceptionBase {
private:
    /* Named _clone to avoid ambiguity with Exception::clone. */
    static Ref<Exception> _clone (const Exception &exc)
    {
	/* This hack is a necessary evil to implement
	 * clone() method for exceptions without imposing
	 * a requirement of implementing this mode
	 * explicitly for each exception class. */

	return grab (static_cast <Exception*> (new T (static_cast <const T&> (exc))));
    }

    static void _raise (const Exception &_self)
    {
	throw T (static_cast <const T&> (_self));
    }

protected:
    ExceptionBase (const ExceptionBase &);

    ExceptionBase ();
};

/* NOTE: Don't derive from ExceptionBase<Exception>, initialization
 * order will be invalid in this case according to the C++ standard.
 */
/*c Base class for all exceptions.
 *
 * Each exception object has a string message and
 * a cause-exception associated with it.
 */
class Exception : public virtual SimplyReferenced
{
public:
    typedef Ref<Exception> (*CloneMethod) (const Exception &);
    typedef void (*RaiseMethod) (const Exception &);

private:
    CloneMethod cloneMethod;
    RaiseMethod raiseMethod;

protected:
    Ref<String>    message;
    Ref<Exception> cause;

    Ref<String>    backtrace;

private:
    Exception& operator = (const Exception &);

    static Ref<Exception> _clone (const Exception &exc)
    {
	return grab (new Exception (exc));
    }

    static void _raise (const Exception &_self)
    {
	throw Exception (_self);
    }

public:
    /* Used by ExceptionBase, should not be used in other places. */
    void setCloneMethod (CloneMethod cloneMethod)
    {
	this->cloneMethod = cloneMethod;
    }

    /* Used by ExceptionBase, should not be used in other places. */
    void setRaiseMethod (RaiseMethod raiseMethod)
    {
	this->raiseMethod = raiseMethod;
    }

    /*m Returns the message associated with this exception object. */
    Ref<String> getMessage () const;

    /*m Returns the cause-exception associated with this exception object. */
    Ref<Exception> getCause () const
    {
	return cause;
    }

    Ref<String> getBacktrace () const
    {
	return backtrace;
    }

    /*m Returns an exact copy of this exception.
     *
     * Exception object's copying constructor is resposible
     * for creating an exact copy of the exception. */
    Ref<Exception> clone () const
    {
	abortIf (cloneMethod == NULL);
	return cloneMethod (*this);
    }

    void raise () const
    {
	abortIf (cloneMethod == NULL);
	raiseMethod (*this);
    }

    Exception (const Exception &exc)
	: M::Referenced (exc),
	  SimplyReferenced (exc)
    {
	cloneMethod = _clone;
	raiseMethod = _raise;

	message   = exc.message;
	cause     = exc.cause;
	backtrace = exc.backtrace;
    }

    /*m The constructor.
     *
     * @message A message to be associated with this exception.
     * @cause A cause-exception to be associated with this exception. */
    Exception (String *message = String::nullString (),
	       Exception *cause = NULL);
};

template <class T>
ExceptionBase<T>::ExceptionBase (const ExceptionBase<T> &)
{
    // Casting to T* while constructor for T has not yet been called feels
    // like the edge of undefined behavior.
    (static_cast <Exception*> (static_cast <T*> (this)))->setCloneMethod (_clone);
    (static_cast <Exception*> (static_cast <T*> (this)))->setRaiseMethod (_raise);
}

template <class T>
ExceptionBase<T>::ExceptionBase ()
{
    (static_cast <Exception*> (static_cast <T*> (this)))->setCloneMethod (_clone);
    (static_cast <Exception*> (static_cast <T*> (this)))->setRaiseMethod (_raise);
}

}

#endif /* __MYCPP__EXCEPTION_H__ */

