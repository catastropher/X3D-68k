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

#define USE_TI92PLUS
#define USE_TI89
#define EXECUTE_IN_GHOST_SPACE

#include "X3D.h"

void x3d_main() {  
#if 0
  if(!x3d_init()) {
    ST_helpMsg(x3d->error.msg);
  }
  
  TRY
    x3d->error.throw_error(1, "Error test!");
  ONERR
    ST_helpMsg(x3d->error.msg);
  ENDTRY
#endif

  x3d_memory_free(NULL);
  ngetchx();
}

