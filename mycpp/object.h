/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2010 Dmitry Shatrov
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

#ifndef __MYCPP__OBJECT_H__
#define __MYCPP__OBJECT_H__

#include <mycpp/types.h>
#include <mycpp/referenced.h>
#include <mycpp/atomic_pointer.h>
#include <mycpp/atomic_int.h>
#include <mycpp/base_util.h>

namespace MyCpp {

/*c General abstraction for MyCpp object system.
 *
 * Currently the only requirement for MyCpp objects is that
 * they must inherit from class <t>Referenced</t>. More requirements
 * can be added in the future, hence this separate class exists. */
class Object : public virtual Referenced
{
public:
#if 0
NO DISPOSE
    /*t Dispose callback.
     *
     * @data User data. */
    typedef void (*DisposeHandler) (void *data);
#endif

    /* Uids are used for comparing object pointers. Acoording to the C++
     * standard, only equality operators are generally applicable to
     * object pointers. Uids allow for using relational operators as well.
     * This is needed, for example, to construct an AVL tree of a set of
     * objects. Uids allow to avoid spreading implementation-defined casts
     * all over the code.
     *
     * Note that this mechanism is specific to MyCpp::Object.
     * Uids are not intended to provide a general way of comparing pointers.
     * Uids are to be used specifically to compare identities of instances of
     * MyCpp::Object.
     *
     * Despite seeming simplicity, the following implementation of Uids is
     * C++ implementation-dependent. On some architectures and/or compilers
     * a heavier hand-written implementation would be required. That's why
     * Uids should NOT be made a general mechanism required to be applicable,
     * for example, to all SimplyReferenced objects. Object class is a natural
     * place for such facility, since it is meant to support "heavy" objects.
     */
    /* TODO Declare standard comparators for Uids. */
    // Note: class Uid may be used as a comparator
    class Uid
    {
	friend class Object;

    protected:
	unsigned long value;

	Uid (const void *ptr)
	{
	    /* TODO Check that this reinterpret_cast is valid,
	     * add a reference to the standard. */
	    /* NOTE: This is C++ implementation-defined. */
	    value = reinterpret_cast <unsigned long> (ptr);
	}

    public:
	static bool greater (const Uid &left,
			     const Uid &right)
	{
	    return left.value > right.value;
	}

	static bool equals (const Uid &left,
			    const Uid &right)
	{
	    return left.value == right.value;
	}
    };

protected:
#if 0
NO DISPOSE
    struct DisposeRecord
    {
	DisposeHandler  disposeHandler;
	void           *data;
	DisposeRecord  *next;
    };

    struct DisposeSlave
    {
	Ref<SimplyReferenced>  ref;
	DisposeSlave          *next;
    };

    DisposeRecord *firstDisposeRecord;
    AtomicPointer /* DisposeSlave* */ firstDisposeSlave;

    void _dispose ();
#endif

public:
#if 0
NO DISPOSE
    /* NOTE I don't think that making addDispose* methods public
     * could hurt in any way. There is at least one case (remoted)
     * when I wanted addDisposeSlave() to be public,
     * and for addDisposeHandler() there was no such need so far. */

    /*m
     *
     * Currently, the queue of dispose handlers is
     * not synchronised nor atomic. That means that addDisposeHandler()
     * should not be called while _dispose() is in progress.
     * In general, the recomendation is to use addDisposeHandler()
     * from within constructors only. This is done because
     * _dispose() is a frequent operation, and using atomic
     * operations in it may be quite expensive.
     */
    void addDisposeHandler (DisposeHandler  handler,
			    void           *data);

    /*m*/
    void addDisposeSlave (SimplyReferenced *slave);
#endif

    static Uid uidFor (Object const *obj)
    {
	return Uid (obj);
    }

    Uid getUid () const
    {
	return uidFor (this);
    }

private:
    Object& operator = (Object const &);
    Object (Object const &);

public:
    Object ()
    {
#if 0
NO DISPOSE
	firstDisposeRecord = NULL;
	firstDisposeSlave.set (NULL);
#endif
    }

#if 0
NO DISPOSE
    ~Object ();
#endif
};

}

/* Includes for API */
#include <mycpp/ref.h>
#include <mycpp/weak_ref.h>
/* (End of includes for API) */

#endif /* __MYCPP__OBJECT_H__ */

