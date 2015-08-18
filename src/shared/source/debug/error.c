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
 
#include "debug/X3D_error.h"
#include "X3D_engine.h"

#include <stdarg.h>

/**
 * Throws an error with the given code and calls the installed handler
 *    function. An optional message (in printf style) can be provided, which
 *    is also passed to the handler.
 *
 *
 * @param code  - error code
 * @param msg   - error message
 *
 * @note A custom handler can be installed by using
 *    @ref x3d_set_error_handler().
 *
 * @note This function does not return, but it is the handler's responsibility
 *    to call exit().
 *
 * @note The default handler is @ref x3d_default_error_handler()
 */
void x3d_error(uint16 code, const char* format, ...) {
  char msg[256];

  va_list list;

  va_start(list, format);
  vsprintf(msg, format, &list);

  // Todo: reset screen

  x3d_get_active_context()->error_handler(code, msg);
}

