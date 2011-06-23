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

#ifndef __MYCPP__SIMPLY_REFERENCED_H__
#define __MYCPP__SIMPLY_REFERENCED_H__

#include <cstdlib> // DEBUG
#include <cstdio>  // DEBUG

#include <mycpp/mycpp_config.h>
#include <mycpp/atomic_int.h>

#ifdef MYCPP_ENABLE_MEMPROF
#include <mycpp/mem_prof_link.h>
#endif

#define SIMPLY_REFERENCED_DEBUG(a) ;

namespace MyCpp {

/* NOTE If one allocates a SimplyReferenced object on the stack,
 * then the number of invocations of unref() for that object
 * must be always less than the number of invocations of ref().
 */
/*c*/
class SimplyReferenced
{
protected:
    AtomicInt refCount;

#ifdef MYCPP_ENABLE_MEMPROF
    MemProfLink mem_prof_link;
#endif

    static AtomicInt num_objects;

    virtual bool sub_last_unref ()
    {
	return true;
    }

public:
    int getNumObjects ()
    {
	return num_objects.get ();
    }

    /*m Increments the reference count of the object.
     * 
     * This operation is atomic. */
    void ref ()
    {
	refCount.inc ();

      SIMPLY_REFERENCED_DEBUG (
	if (traced.get ())
	    fprintf (stderr, "reftrace: ref:   %lx\n", (unsigned long) this);

	if (crash_on_ref.get ()) {
	    fprintf (stderr, "crashing on ref()\n");
	    abort ();
	}
      ) /* (DEBUG) */
    }

    /*m Decrements the reference count of the object.
     *
     * This operation is atomic.
     *
     * TODO Add a note on _sub_last_unref (). */
    void unref ();

    // FIXME This should be forbidden.
    SimplyReferenced& operator = (SimplyReferenced const &)
    {
	/* Reference count should not change on assignment. */
	return *this;
    }

    // FIXME This should be forbidden.
    SimplyReferenced (SimplyReferenced const &)
    {
	refCount.set (1);
	num_objects.add (1);
    }

    /* NOTE! This method should be used for debugging purposes only. */
    int getRefCount () const
    {
	return refCount.get ();
    }

SIMPLY_REFERENCED_DEBUG (
private:
    AtomicInt traced;
    AtomicInt crash_on_ref;

public:
    void setTraced (bool traced)
    {
	this->traced.set (traced ? 1 : 0);
    }

    void setCrashOnRef (bool crash)
    {
	this->crash_on_ref.set (crash ? 1 : 0);
    }
) /* (DEBUG) */

#ifdef MYCPP_ENABLE_MEMPROF
    void setMemProfLink (MemProfLink const &link)
    {
	abortIf (!mem_prof_link.isNull ());
	this->mem_prof_link = link;
    }
#endif

    SimplyReferenced ()
	/* NOTE! refCount _must_ be initialized to "1", because
	 * it is (usually) legal to call arbitrary functions
	 * accepting a pointer to the object during its construction,
	 * and those functions are free to bind a Ref to this objec.
	 * If refCount is initialized to 0, then once that Ref gets
	 * destroyed, the refCount will drop to 0 triggering deletion
	 * of the object, with a (likely) segfault afterwards. */
	: refCount (1)
    {
	num_objects.add (1);
    }

    virtual ~SimplyReferenced ()
    {
	num_objects.add (-1);
#ifdef MYCPP_ENABLE_MEMPROF
	// TEST
#error
	removeMemProfEntry (mem_prof_link);
#endif
    }
};

}

/* Includes for API */
#include <mycpp/ref.h>
/* (End of includes for API) */

#endif /* __MYCPP__SIMPLY_REFERENCED_H__ */

