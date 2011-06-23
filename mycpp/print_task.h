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

#ifndef __MYCPP__PRINT_TASK_H__
#define __MYCPP__PRINT_TASK_H__

#include <mycpp/string.h>
#include <mycpp/file.h>
#include <mycpp/base_util.h>
#include <mycpp/strutil.h>

// For user applications include io.h, not this header.

namespace MyCpp {

template <class T> class Pr_Base;

class PrintTask
{
protected:
    // We're using an ad-hoc linked list here. The source of complexity
    // is that PrintTask objects are supposed to be created on the stack
    // as temporal objects. Such objects can only be passed to other
    // functions by constant references, which doesn't allow to write
    // code like:
    //
    //     PrintTask& add (const PrintTask &prv_task) {
    //         prv_task->next = this;
    //         /* ... */
    //     }
    //
    // const_cast is inappropriate in this case because of compiler optimizations.
    //
    // Provided this, I've chosen to return new PrintTask objects by value from the add()
    // method. This means that we don't know at the time of their creation
    // what copy of it will be the next in the chain. Hence we delay setting
    // of 'previous->next' pointer till the next add() method call.
    //
    // In the end, we get a linked list with one past last element's
    // 'next' pointer pointing to the void. But that's not a problem since
    // we always start list traversals from the last element.
    //
    // The last element's 'next' pointer serves as a pointer to the first
    // element of the list.

    PrintTask *next,
	      *previous;

public:
    virtual void out (File *file) const
	       throw (IOException,
		      InternalException) = 0;

    virtual void print (File *file) const = 0;

    virtual unsigned long length () const = 0;

    unsigned long chainLength () const
    {
	unsigned long len = 0;

	PrintTask *cur = next;
	for (;;) {
	    len += cur->length ();
	    cur = cur->next;
	    if (cur == next)
		break;
	}

	if (next != this)
	    len += length ();

	return len;
    }

    void chainOut (File *file) const
	    throw (IOException,
		   InternalException)
    {
	PrintTask *cur = next;
	for (;;) {
	    cur->out (file);
	    cur = cur->next;
	    if (cur == next)
		break;
	}

	if (next != this)
	    out (file);
    }

    void chainPrint (File *file) const
    {
	PrintTask *cur = next;
	for (;;) {
	    cur->print (file);
	    cur = cur->next;
	    if (cur == next)
		break;
	}

	if (next != this)
	    print (file);
    }

    template <class T>
    Pr_Base<T> add (const Pr_Base<T> &task)
    {
	if (previous != NULL)
	    previous->next = this;

	return Pr_Base<T> (task, this);
    }

    template <class T>
    Pr_Base<T> add (const T &value)
    {
	if (previous != NULL)
	    previous->next = this;

	return Pr_Base<T> (value, this);
    }

    template <class T>
    Pr_Base<T> operator () (const Pr_Base<T> &task)
    {
	return add (task);
    }

    template <class T>
    Pr_Base<T> operator () (const T &value)
    {
	return add (value);
    }

    template <size_t N> Pr_Base<const char*> operator () (const char (&value) [N]);

    template <class T>
    Pr_Base<T> operator << (const T &value)
    {
	return add (value);
    }

    template <size_t N> Pr_Base<const char*> operator << (const char (&value) [N]);

    PrintTask (PrintTask *previous)
    {
	abortIf (previous == NULL);

	this->previous = previous;
	next = previous->next;
    }

    PrintTask ()
    {
	next = this;
	previous = NULL;
    }

    virtual ~PrintTask ()
    {
    }
};

template<class T>
class Pr_Base : public PrintTask
{
protected:
    // Note: no references here! Store by value,
    // because Pr_Base may be copied multiple times,
    // so that the scope of the original value may end.
    T value;

public:
    void out (File *file) const
	throw (IOException,
	       InternalException)
    {
	file->out (value);
    }

    void print (File *file) const
    {
	file->print (value);
    }

    unsigned long length () const
    {
	return File::nout (value);
    }

    Pr_Base (T const &value)
	: value (value)
    {
    }

    Pr_Base (Pr_Base<T> const &task, PrintTask *previous)
	: PrintTask (previous),
	  value (task.value)
    {
    }

    Pr_Base (T const &value, PrintTask *previous)
	: PrintTask (previous),
	  value (value)
    {
    }
};

template <size_t N>
Pr_Base<const char*> PrintTask::operator () (const char (&value) [N])
{
    return add ((const char*) value);
}

template <size_t N>
Pr_Base<const char*> PrintTask::operator << (const char (&value) [N])
{
    return add ((const char*) value);
}

// Support for syntaxes:
//
//     Pt (1) (2.0) ("3")
//
//     Pt (1) << 2.0 << "3"
//
template <class T>
Pr_Base<T> Pt (T value)
{
    return Pr_Base<T> (value);
}

#if 0
template <>
Pr_Base<const char*> Pt<const char*> (const char *value)
{
    return Pr_Base<const char*> (value);
}

template <>
Pr_Base<const Byte*> Pt<const Byte*> (const Byte *value)
{
    return Pr_Base<const Byte*> (value);
}
#endif

// FIXME This doesn't work
//
// Support for syntax:
//
//     Pc() << 1 << 2.0 << "3"
//
// I don't use "PrintTask Pt()" function beacause of gcc warnings
// about unused static inline function Pt() for files which include
// print_task.h but do not call Pt().
//
class Pc : public PrintTask
{
public:
    virtual void out (File * /* file */) const
	       throw (IOException,
		      InternalException)
    {
	// No-op
    }

    virtual void print (File * /* file */) const
    {
	// No-op
    }

    virtual unsigned long length () const
    {
	return 0;
    }
};

// The idea with "PrintTaskFactory Pr;" global object is no good.
// It is here for backwards compatibility and is scheduled for removal.
class PrintTaskFactory
{
public:
    template <class T>
    Pr_Base<T> operator () (T value) const
    {
	return Pr_Base<T> (value);
    }

    Pr_Base<ConstMemoryDesc> operator () (const char *str) const
    {
	if (str != NULL)
	    return Pr_Base<ConstMemoryDesc> (ConstMemoryDesc (str, countStrLength (str)));
	else
	    return Pr_Base<ConstMemoryDesc> (ConstMemoryDesc ());
    }

    Pr_Base<ConstMemoryDesc> operator () (const String *str) const
    {
	if (str != NULL)
	    return Pr_Base<ConstMemoryDesc> (str->getMemoryDesc ());

	return Pr_Base<ConstMemoryDesc> (ConstMemoryDesc ());
    }

    template <class T>
    Pr_Base<T> operator << (T value) const
    {
	return Pr_Base<T> (value);
    }
};

extern PrintTaskFactory Pr;

}

#endif /* __MYCPP__PRINT_TASK_H__ */

