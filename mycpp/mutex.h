/* MyCpp - MyNC C++ helper library
 * Copyright (C) 2006-2010  Dmitry M. Shatrov
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

#include <mycpp/atomic_int.h>
#include <mycpp/base_util.h>

namespace MyCpp {

/*c */
class MutexBase
{
protected:
    /* GMutex* */
    void *gmutex;

    /* Introduced to catch locked mutexes being destroyed
     * and to enable nice locking sanity assertions (hence atomic). */
    AtomicInt locked;

    // This method is deprecated. It has never been used, hence it's private now.
    bool isLocked ();

public:
    /*m Locks this mutex. */
    void lock ();

    /*m Unlocks this mutex. */
    void unlock ();

    // May be a no-op depending on MyCpp config.
    void assertLocked ();

    // May be a no-op depending on MyCpp config.
    void assertUnlocked ();

    /* For internal use only:
     * should not be expected to be present in future versions.
     *
     * Returns a GMutex*. */
    void* get_glib_mutex ();

    MutexBase ();

    ~MutexBase ();
};

template <class Base = EmptyBase>
class Mutex_ : public MutexBase,
	       public Base
{
};

typedef Mutex_<> Mutex;

class MutexLock
{
protected:
    Mutex &mutex;
    bool locked;

private:
    MutexLock& operator = (MutexLock const &);
    MutexLock (MutexLock const &);

public:
    void lock ()
    {
	abortIf (locked);
	mutex.lock ();
	locked = true;
    }

    void unlock ()
    {
	abortIf (!locked);
	mutex.unlock ();
	locked = false;
    }

    MutexLock (Mutex &mutex)
	: mutex (mutex)
    {
	mutex.lock ();
	locked = true;
    }

    ~MutexLock ()
    {
	if (locked)
	    mutex.unlock ();
    }
};

class MutexUnlock
{
protected:
    Mutex &mutex;

private:
    MutexUnlock& operator = (MutexUnlock const &);
    MutexUnlock (MutexUnlock const &);

public:
    MutexUnlock (Mutex &mutex)
	: mutex (mutex)
    {
	mutex.unlock ();
    }

    ~MutexUnlock ()
    {
	mutex.lock ();
    }
};

}


#endif /*__MYCPP__MUTEX_H__*/

