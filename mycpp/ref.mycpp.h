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

#ifndef __MYCPP__REF_H__
#define __MYCPP__REF_H__

#include <mycpp/mycpp_config.h>
#include <mycpp/simply_referenced.h>
#include <mycpp/base_util.h>

#ifdef MYCPP_ENABLE_MEMPROF
#include <typeinfo>
#include <mycpp/mem_prof_link.h>
#endif

namespace MyCpp {

/* Robust takeover guarantees that references which have been
 * taken away are not dereferenced until reassignment. This comes
 * at the cost of one additional 'bool' field per reference and
 * some extra assignment operations.
 *
 * The alternative is to simply nullify references on takeover.
 * This is more efficient but less safe, because misuse of
 * such references will not be detected during execution. */
#define MYCPP_REF_ROBUST_TAKEOVER 1

/*c <t>Ref</t> objects provide a convinient method for
 * managing <t>SimplyReferenced</t> objects.
 *
 * A <t>Ref</t> object holds a reference to a <t>SimplyReferenced</t>
 * object. The object is "grabbed" (by calling the <c>grab</c> method)
 * when the reference is associated with that object,
 * and the object is "unrefed" (by calling the <c>unref</c> method)
 * when the <c>Ref</c> is destroyed, or when it gets associated
 * to another object. */
/* TODO template <class T> class TakeOverRef : public Ref<T> */
template <class T>
class Ref
{
    // TODO friend template
public: // fictive "public" to allow member functions of Ref<A> operate on data of Ref<B>
/* protected: */
    // TODO Why mutable? Describe it.
    mutable T *obj;

#ifdef MYCPP_REF_ROBUST_TAKEOVER
    /* References which have been taken away can not be
     * dereferenced unil reassignment, because such references
     * do not guarantee that the object will not be destroyed
     * meanwhile. */
    mutable bool takenAway;
#endif

public:
    /*m Returns a pointer to associated object.
     *
     * If the <c>Ref</c> is not (yet) assinged to any object,
     * then <i>NULL</i> is returned. */
    T* ptr () const
    {
#ifdef MYCPP_REF_ROBUST_TAKEOVER
	if (takenAway)
	    abortIfReached ();
#endif

	return obj;
    }

    T& der () const
    {
	if (obj == NULL)
	    abortIfReached ();

	return *obj;
    }

    /*m Tells whether the <c>Ref</c> is associated with any object.
     *
     * Returns <c>true</c>, if this <c>Ref</c> is associated
     * with an object, otherwise returns <c>false</c>. */
    bool isNull () const
    {
#ifdef MYCPP_REF_ROBUST_TAKEOVER
	if (takenAway)
	    abortIfReached ();
#endif

	return obj == NULL;
    }

#if 0
// Unnecessary
    template <class C>
    operator C* () const
    {
#ifdef MYCPP_REF_ROBUST_TAKEOVER
	if (takenAway)
	    abortIfReached ();
#endif

	return static_cast <C*> (obj);
    }
#endif

    /*m Converts <c>Ref</c> to a pointer to associated object.
     *
     * Returns <i>NULL</i>, if there is no object associated
     * with this <c>Ref</c>. */
    operator T* () const
    {
#ifdef MYCPP_REF_ROBUST_TAKEOVER
	if (takenAway)
	    abortIfReached ();
#endif

	return obj;
    }

#if 0
    operator T const * () const
    {
	return obj;
    }

    // for NULL
    operator int () const
    {
	abortIf (obj != NULL);
	return NULL;
    }
#endif

    // TODO ?
    operator Ref<T const> () const
    {
	return obj;
    }

    /*m Allows to use the <c>Ref</c> object as a pointer to
     * associated object.
     *
     * Generally, one should not use this operator, if
     * there is no associated object for this <c>Ref</c>. */
    T* operator -> () const
    {
#ifdef MYCPP_REF_ROBUST_TAKEOVER
	if (takenAway)
	    abortIfReached ();
#endif

	return obj;
    }

    T& operator * () const
    {
	abortIf (obj == NULL);
	return *obj;
    }

#if 0
    /*m Dereferences <c>Ref</c>, providing a C++ reference
     * to associated object.
     *
     * One <i>must not</i> use this operation, if there is
     * no associated object fot this <c>Ref</c>. */
    T& operator * () const {
	return *obj;
    }
#endif

#if 0
// 09.08.19
// takeOver() is now forbidden, because it uses const_cast, which is unsafe.
// (the ban was introduced after a problem with PrintTask being optimized out by gcc)

    template <class C>
    void takeOver (const Ref<C> &_ref)
    {
	/* FIXME This is questionable! What if the reference passed
	 * is actually a const object? */
	Ref<C> &ref = const_cast <Ref<C>&> (_ref);

	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->unref ();

	obj = static_cast <T*> (ref.obj);

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
	ref.takenAway = true;
#endif

	ref.obj = NULL;
    }
#endif

    void setNoRef (T *obj)
    {
	if (this->obj != NULL)
	    static_cast <SimplyReferenced*> (this->obj)->unref ();

	this->obj = obj;

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif
    }

#if 0
    void dropObject ()
    {
	obj = NULL;

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takeAway = true;
#endif
    }
#endif

    /*m Makes the object associated with <i>ref</i>
     * being associated with this <c>Ref</c> as well. */
    template <class C>
    Ref& operator = (const Ref<C> &ref)
    {
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->unref ();

	obj = static_cast <T*> (ref.obj);
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif

	return *this;
    }

    Ref& operator = (const Ref &ref)
    {
	if (this == &ref)
	    return *this;

	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->unref ();

	obj = ref.obj;
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif

	return *this;
    }

    /*m Associates object <i>obj</i> with this <c>Ref</c>.
     *
     * If <i>obj</i> is <i>NULL</i>, then this <c>Ref</c>
     * becomes a null reference. */
    template <class C>
    Ref& operator = (C *obj)
    {
	if (this->obj != NULL)
	    static_cast <SimplyReferenced*> (this->obj)->unref ();

	this->obj = static_cast <T*> (obj);
	if (this->obj != NULL)
	    static_cast <SimplyReferenced*> (this->obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif

	return *this;
    }

    // This is here just for "= NULL" (NULL is an integer in C++).
    Ref& operator = (T *obj)
    {
//	return operator = <T> (obj);

	if (this->obj != NULL)
	    static_cast <SimplyReferenced*> (this->obj)->unref ();

	this->obj = obj;
	if (this->obj != NULL)
	    static_cast <SimplyReferenced*> (this->obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif

	return *this;
    }

    /* TODO If Ref can be casted to T*, then probably one of these methods
     * is unnecessary. */
    /*m Makes a copy of <i>ref</i>.
     *
     * After completion, the object associated with
     * <i>ref</i> is also associated with the returned <c>Ref</c>. */
    template <class C>
    Ref (const Ref<C> &ref)
    {
	obj = static_cast <T*> (ref.obj);
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif
    }

    Ref (const Ref &ref)
    {
	obj = ref.obj;
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif
    }

    /*m Creates a <c>Ref</c>, and associates it with <i>obj</i>. */
    template <class C>
    Ref (C *obj)
    {
	this->obj = static_cast <T*> (obj);
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif
    }

    // This is here just for "= NULL" (NULL is an integer in C++).
    Ref (T *obj)
    {
	this->obj = obj;
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->ref ();

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif
    }

#if 0
This was a terrible idea. It is now replaced by "reference masters"
in SimplyReferenced class (slightly better).

    /*m Creates a "delegated" <c>Ref</c>.
     *
     * That is, <c>ptr()</c> and
     * <c>operator -></c> will return <c>obj</c> pointer, but actual
     * refcounting will be performed on <c>rd</c> object. This
     * allows to return references to instances of nested classes which
     * depend on instances of their corresponding containee classes,
     * without introducing cyclic references. Consider the following
     * example:
     *
     * One can write:
     * <code>
     ^ <![CDATA[
     | class Aggregate {
     | public:
     |     class Iface : public Interface {
     |         Aggregate *agr;
     |     };
     |
     |     Ref<Iface> iface;
     |
     |     Ref<Interface> getInterface () {
     |         return Ref<Interface> (iface, this);
     |     }
     | };
     v ]]>
     * </code>
     *
     * ...instead of:
     * <code>
     ^ <![CDATA[
     | class Aggregate {
     | public:
     |     class Iface : public Interface {
     |         Ref<Aggregate> agr;
     |     };
     |
     |     Ref<Iface> iface;
     |
     |     Ref<Interface> getInterface () {
     |         return Ref<Interface> (iface);
     |     }
     | };
     v ]]>
     * </code>
     *
     * ...and reference counting will still be sane,
     * but the <t>Aggregate</t><tt>&lt;---&gt;</tt><t>Iface</t>
     * reference loop will be avoided.
     *
     * @obj An object to be associated with this reference.
     * @rd  A <t>SimplyReferenced</t> to perform reference counting on. */
    Ref (T *obj, SimplyReferenced *rd) {
	    this->obj = obj;
	    this->rd  = rd;
	    if (rd != NULL)
		/* FIXME Why grab(), and not ref()? */
		rd->grab ();
    }
#endif

    /*m Creates a null <c>Ref</c>. */
    Ref ()
    {
	obj = NULL;

#ifdef MYCPP_REF_ROBUST_TAKEOVER
	takenAway = false;
#endif
    }

    ~Ref ()
    {
	if (obj != NULL)
	    static_cast <SimplyReferenced*> (obj)->unref ();
    }
};

#ifdef MYCPP_ENABLE_MEMPROF
// mem_prof version
template <class T>
Ref<T> _grab (T *obj,
	      const char *source_file,
	      unsigned long source_line)
{
    abortIf (obj == NULL);

    const char *backtrace = rawCollectBacktrace ();
    obj->setMemProfLink (addMemProfEntry (obj,
					  source_file,
					  source_line,
					  typeid (T).name (),
					  backtrace));
    if (backtrace != NULL)
	delete[] backtrace;

    Ref<T> ref;
    ref.setNoRef (obj);
    return ref;
}

#define grab(obj) _grab (obj, __FILE__, __LINE__)
#else
// non-mem_prof version
template <class T>
Ref<T> grab (T *obj)
{
    abortIf (obj == NULL);
    Ref<T> ref;
    ref.setNoRef (obj);
    return ref;
}
#endif

#if 0
// Forbidden (see above)

template <class T, class C>
Ref<T> takeOver (const Ref<C> &ref)
{
    Ref<T> new_ref;
    new_ref.takeOver (ref);
    return new_ref;
}
#endif

}

#endif /* __MYCPP__REF_H__ */

