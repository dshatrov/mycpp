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

#ifndef __MYCPP__DATA_MUTEX_H__
#define __MYCPP__DATA_MUTEX_H__

#include <mycpp/mutex.h>
#include <mycpp/simply_referenced.h>

namespace MyCpp {

class DataMutexBase
{
protected:
    Mutex mutex;

public:
    void lock ();

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
};

template <class Base = EmptyBase>
class DataMutex_ : public DataMutexBase,
		   public Base
{
};

typedef DataMutex_<> DataMutex;

class DataMutexLock
{
protected:
    DataMutex &mutex;
    bool locked;

private:
    DataMutexLock& operator = (DataMutexLock const &);
    DataMutexLock (DataMutexLock const &);

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

    DataMutexLock (DataMutex &mutex)
	: mutex (mutex)
    {
	mutex.lock ();
	locked = true;
    }

    ~DataMutexLock ()
    {
	if (locked)
	    mutex.unlock ();
    }
};

class DataMutexUnlock
{
protected:
    DataMutex &mutex;

private:
    DataMutexUnlock& operator = (DataMutexUnlock const &);
    DataMutexUnlock (DataMutexUnlock const &);

public:
    DataMutexUnlock (DataMutex &mutex)
	: mutex (mutex)
    {
	mutex.unlock ();
    }

    ~DataMutexUnlock ()
    {
	mutex.lock ();
    }
};

}

#endif /* __MYCPP__DATA_MUTEX_H__ */

