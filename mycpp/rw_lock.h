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

#ifndef __MYCPP__RW_LOCK_H__
#define __MYCPP__RW_LOCK_H__

/* This include directive makes gthread.h included in virtually any
 * MyCpp-aware source file. */
#include <glib/gthread.h>

#include <mycpp/types.h>
#include <mycpp/base_util.h>

namespace MyCpp {

class RwLockBase
{
protected:
    GStaticRWLock lock;

public:
    void readerLock ()
    {
	g_static_rw_lock_reader_lock (&lock);
    }

    bool readerTrylock ()
    {
	return (bool) g_static_rw_lock_reader_trylock (&lock);
    }

    void readerUnlock ()
    {
	g_static_rw_lock_reader_unlock (&lock);
    }

    void writerLock ()
    {
	g_static_rw_lock_writer_lock (&lock);
    }

    bool writerTrylock ()
    {
	return (bool) g_static_rw_lock_writer_trylock (&lock);
    }

    void writerUnlock ()
    {
	g_static_rw_lock_writer_unlock (&lock);
    }

    RwLockBase ()
    {
	if (!gthreadInitialized ())
	    abortIfReached ();

	g_static_rw_lock_init (&lock);
    }

    ~RwLockBase ()
    {
	g_static_rw_lock_free (&lock);
    }
};

template <class Base = EmptyBase>
class RwLock_ : public RwLockBase,
		public Base
{
};

typedef RwLock_<> RwLock;

}

#endif /* __MYCPP__RW_LOCK_H__ */

