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

#ifndef __MYCPP__MYCPP_THREAD_LOCAL_H__
#define __MYCPP__MYCPP_THREAD_LOCAL_H__

#include <mycpp/simply_referenced.h>
#include <mycpp/list.h>

namespace MyCpp {

class MyCpp_ThreadLocal
{
public:
    List< Ref<SimplyReferenced> > deletion_queue;

    /* Branch layers are being popped out one at a time.
     * When saving is performed, all branch layers from the
     * topmost saving group are saved as a single branch layer,
     * which is the result of merging of all layers from
     * the topmost saving group.
     *
     * The inner list contains branch layers for the same
     * saving group. The outer list is the list of saving
     * groups. Example:
     *
     *     +--------------------------+
     *     | topmost branch layer     |
     *     +--------------------------+
     *     | 2nd-level branch layer,  |
     *     | in the same saving group |
     *     | as the first one         |
     *     +--------------------------+
     *     | 3rd branch layer from    |
     *     | the 1st saving group     |
     *     +==========================+
     *     | 4th branch layer,        |
     *     | 2nd saving group         |
     *     +==========================+
     *     | 5th branch layer,        |
     *     | 3rd saving group         |
     *     +==========================+
     *
     * Branch layers are lists of references to SimplyReferenced objects.
     */
    List< List< List< Ref<SimplyReferenced> > > > branch_layers;

    // Only one data mutex is allowed to be locked at a time by a given thread,
    // hence the value of this counter should be either 0 or 1.
    unsigned long data_mutex_counter;

    MyCpp_ThreadLocal ();
};

// Never returns NULL.
MyCpp_ThreadLocal* myCpp_getThreadLocal ();

void threadLocal_myCppInit ();

}

#endif /* __MYCPP__MYCPP_THREAD_LOCAL_H__ */

