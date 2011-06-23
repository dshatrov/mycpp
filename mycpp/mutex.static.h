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

#ifndef __MYCPP__MUTEX_H__
#define __MYCPP__MUTEX_H__

#include <glib.h>

#include <mycpp/types.h>
#include <mycpp/simply_referenced.h>
#include <mycpp/string.h>
#include <mycpp/util.h>

namespace MyCpp {

/* TODO 07.08.05
 * Using GStaticMutex is indeed a nicer solution,
 * since this way global mutex variables are allowed
 * (they can be constructed before a call to g_thread_init).
 * The problem is in odd warnings that gcc 4.x.x
 * (my version is 4.1.2-0ubuntu4) outputs for g_static_mutex_* macros.
 * Probably it is glib that needs fixing. */

/*c Mutex - a primitive for thread synchronization. */
class Mutex : public virtual SimplyReferenced
{
protected:
    GStaticMutex mutex;

    /* Introduced to catch locked mutexes being destroyed. */
    bool locked;

public:
    /*m Locks this mutex. */
    void lock () {
	g_static_mutex_lock (&mutex);
	locked = true;
    }

    /*m Unlocks this mutex. */
    void unlock () {
	locked = false;
	g_static_mutex_unlock (&mutex);
    }

    /* For internal use only:
     * should not be expected to be present in future versions */
    GStaticMutex* get_glib_mutex () {
	return &mutex;
    }

    Mutex () {
	if (!gthreadInitialized ())
	    abortIfReached ();

	locked = false;
	g_static_mutex_init (&mutex);
    }

    ~Mutex () {
	if (locked)
	    g_critical ("MyCpp.Mutex~(): locked");

	g_static_mutex_free (&mutex);
    }
};

};

#endif /*__MYCPP__MUTEX_H__*/

