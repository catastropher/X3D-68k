/* This file is part of X3D.
 * 
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>

#include "X3D_core.h"

/**
 * Implementation for x3d->error.throw_error().
 * 
 * @param code    - numeric error code
 * @param format  - printf-like format string
 * 
 * @returns Nothing.
 * @note    code cannot be 0!
 * @note    This function does not return.
 */
void x3d_error_throw(int16 code, const char* format, ...) {
  va_list list;
  
  va_start(list, format);
  //vsprintf(x3d->error.msg, format, list);
  
  //x3d->error.code = code;
  //x3d_error_do_throw(code);
}















