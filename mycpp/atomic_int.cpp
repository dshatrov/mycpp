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

#include <glib.h>

#include <mycpp/atomic_int.h>


#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC


// Unnecessary here #define MYCPP__ATOMIC_INT__FAKE_ATOMIC


namespace MyCpp {

void
AtomicInt::set (int value)
{
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    g_atomic_int_set (&this->val, (gint) value);
#else
    val = value;
#endif
}

int
AtomicInt::get () const
{
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    return (int) g_atomic_int_get (&val);
#else
    return val;
#endif
}

void
AtomicInt::inc ()
{
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    g_atomic_int_inc (&val);
#else
    val ++;
#endif
}

void
AtomicInt::add (int a)
{
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    g_atomic_int_add (&val, (gint) a);
#else
    val += a;
#endif
}

bool
AtomicInt::compareAndExchange (int oldValue,
			       int newValue)
{
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    return (bool) g_atomic_int_compare_and_exchange (
			&val, (gint) oldValue, (gint) newValue);
#else
    if (val == oldValue) {
	val = newValue;
	return true;
    }

    return false;
#endif
}

bool
AtomicInt::decAndTest ()
{
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    return (bool) g_atomic_int_dec_and_test (&val);
#else
    val --;
    return val == 0;
#endif
}

AtomicInt::AtomicInt (int val)
{
#ifndef MYCPP__ATOMIC_INT__FAKE_ATOMIC
    g_atomic_int_set (&this->val, (gint) val);
#else
    this->val = val;
#endif
}

}


#endif // MYCPP__ATOMIC_INT__FAKE_ATOMIC


