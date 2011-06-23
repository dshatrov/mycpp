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

#ifndef __MYCPP__SIMPLY_REFERENCED__H__
#define __MYCPP__SIMPLY_REFERENCED__H__


#include <libmary/referenced.h>


namespace MyCpp {

class Referenced;

class SimplyReferenced : public virtual M::Referenced
{
public:
    // This overrides injected-class-name M::Referenced, which conflicts with
    // MyCpp::Referenced.
    typedef MyCpp::Referenced Referenced;
};

}


/* Includes for API */
#include <mycpp/ref.h>
/* (End of includes for API) */


#endif /* __MYCPP__SIMPLY_REFERENCED__H__ */

