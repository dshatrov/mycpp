/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006  Dmitry M. Shatrov
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

#ifndef __MYCPP__ATOMIC_INT_H__
#define __MYCPP__ATOMIC_INT_H__


#ifdef MYCPP__ATOMIC_INT__FAKE_ATOMIC
#error
#endif
//#define MYCPP__ATOMIC_INT__FAKE_ATOMIC


namespace MyCpp {

/*c Atomic integer variable. */
class AtomicInt
{
protected:
    /* Some confusion about gint, because source files including
     * this header should not depend on glib. */
    /* FIXME Make this an actual gint. There is no safe way to avoid that. */
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    volatile int /*gint*/ val;
#else
    int val;
#endif

public:
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    /*m Assign a new value atomically.
     *
     * @val the value to be assigned. */
    void set (int val);

    /*m Atomically get the current value. */
    int get () const;

    void inc ();

    /*m Add an integer value atomically.
     *
     * @a the integer value to add. */
    void add (int a);

    /*m*/
    bool compareAndExchange (int oldValue,
			     int newValue);

    /* Returns 'true' if the integer is 0 after decrementing it. */
    bool decAndTest ();

    AtomicInt (int val = 0);
#else
    void set (int value)
    {
	val = value;
    }

    int get () const
    {
	return val;
    }

    void inc ()
    {
	val ++;
    }

    void add (int a)
    {
	val += a;
    }

    bool compareAndExchange (int oldValue,
			     int newValue)
    {
	if (val == oldValue) {
	    val = newValue;
	    return true;
	}

	return false;
    }

    bool decAndTest ()
    {
	val --;
	return val == 0;
    }

    AtomicInt (int val = 0)
	: val (val)
    {
    }
#endif // MYCPP__ATOMIC_INT__FAKE_ATOMIC
};

}


/* We leave this macro defined for atomic_int.cpp
 *
#ifdef MYCPP__ATOMIC_INT__FAKE_ATOMIC
#undef MYCPP__ATOMIC_INT__FAKE_ATOMIC
#endif
 */


#endif /*__MYCPP__ATOMIC_INT_H__*/

