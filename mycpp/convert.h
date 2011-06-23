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

#ifndef __GCODE_CRIO_CONVERT_H__
#define __GCODE_CRIO_CONVERT_H__

/* Общие для всей системы gcode процедуры разбора
 * чисел из строк
 */

namespace MyCpp {

/* TODO Substitute these with strToDouble() and strToUlong() */
/* Возврацает true, если формат числа правильный */
bool string_to_double (const char *str, double *val);
bool string_to_long (const char *str, long *val);

}

#endif /*__GCODE_CRIO_CONVERT_H__*/

