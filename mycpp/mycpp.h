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

#ifndef __MYCPP__MYCPP_H__
#define __MYCPP__MYCPP_H__

/** .root
 *
 * <toc/>
 * <cat name="uid"/>
 */

// "mycpp.h" provides support for basic MyCpp concepts.

#include <mycpp/mycpp_config.h>

#include <mycpp/types.h>
#include <mycpp/atomic_int.h>
#include <mycpp/atomic_pointer.h>
#include <mycpp/string.h>
#include <mycpp/memory_desc.h>
#include <mycpp/callback_desc.h>
#include <mycpp/uid.h>

#include <mycpp/pointer.h>
#include <mycpp/extractor.h>
#include <mycpp/comparator.h>
#include <mycpp/iterator.h>
#include <mycpp/iteratable.h>
#include <mycpp/cloneable.h>

#include <mycpp/simply_referenced.h>
#include <mycpp/object.h>
#include <mycpp/object_factory.h>
#include <mycpp/ref.h>
#include <mycpp/weak_ref.h>

#include <mycpp/list.h>
#include <mycpp/intrusive_list.h>
#include <mycpp/embedded_list.h>
#include <mycpp/avl_tree.h>
#include <mycpp/intrusive_avl_tree.h>
#include <mycpp/map.h>
#include <mycpp/multi_map.h>
#include <mycpp/buffer.h>
#include <mycpp/linear_buffer_info.h>
#include <mycpp/vstack.h>
#include <mycpp/vslab.h>

#include <mycpp/mutex.h>
#include <mycpp/data_mutex.h>
#include <mycpp/cond.h>
#include <mycpp/rw_lock.h>

#include <mycpp/exception.h>
#include <mycpp/internal_exception.h>
#include <mycpp/io_exception.h>
#include <mycpp/middleware_exception.h>

#include <mycpp/informer.h>

#include <mycpp/util.h>

namespace MyCpp {

#if 0
/* 'data' is of type SimplyReferenced* */
void referencedRefCallback   (void *data);
/* 'data' is of type SimplyReferenced* */
void referencedUnrefCallback (void *data);
#endif

void myCppInit ();

}

#endif /* __MYCPP__MYCPP_H__ */

