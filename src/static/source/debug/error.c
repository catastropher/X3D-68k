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

#include "X3D_fix.h"

/**
 * X3D's default error handler that's called whenever an error occurs.
 *
 * @param code  - error code
 * @param msg   - error message to display
 *
 * @note This function does not return because it calls exit().
 * @note A custom handler can be installed by using
 *    @ref x3d_set_error_handler().
 */
void x3d_default_error_handler(uint16 code, const char* msg) {
  clrscr();
  printf("Error code %d: %s\n", code, msg);
  ngetchx();
  
  exit(-1);
}