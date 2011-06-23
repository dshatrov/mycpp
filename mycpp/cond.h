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

#ifndef __MYCPP__COND_H__
#define __MYCPP__COND_H__

#include <glib.h> // FIXME For GTimeVal

#include <mycpp/types.h>
#include <mycpp/mutex.h>
#include <mycpp/data_mutex.h>

namespace MyCpp {

/*c Condition - a primitive for thread synchronization. */
class CondBase
{
protected:
    /* GCond* */
    void *cond;

public:
    /*m Triggers this condition, causing exactly <b>one</b> of
     * the waiting threads to wake up. */
    void signal ();

    /* Causes <b>all</b> of the waiting threads to wake up. */
    void broadcast ();

    /*m Waits until the current thread is woken up
     * on this condition.
     *
     * A protection mutex is required. The mutex must
     * be locked prior to calling this method.
     *
     * @mutex The protection mutex. */
    void wait (Mutex &mutex);

    void wait (DataMutex &mutex);

    /* FIXME Using GTimeVal in MyCpp is ugly :( */

    /*m Waits until the current thread is woken up
     * or a timeout occurs.
     *
     * A protection mutex is required. The mutex must
     * be locked prior to calling this method.
     *
     * @mutex The protection mutex.
     * @tv A pointer to <t>GTimeVal</t> structure,
     * see glib reference manual for details. */
    bool timedWait (Mutex &mutex,
		    GTimeVal *tv)
    {
	return g_cond_timed_wait ((GCond*) cond, (GMutex*) (mutex.get_glib_mutex ()), tv);
    }

    CondBase ();

    ~CondBase ();
};

template <class Base = EmptyBase>
class Cond_ : public CondBase,
	      public Base
{
};

typedef Cond_<> Cond;

}

#endif /*__MYCPP__COND_H__*/

