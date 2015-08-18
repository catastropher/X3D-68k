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

#include "X3D_config.h"
#include "X3D_fix.h"
#include "resource/X3D_memory.h"
#include "X3D_engine.h"
#include "debug/X3D_log.h"


void* x3d_malloc(X3D_Context* context, uint16 size) {
  printf("Size: %d\n", size);
  x3d_log(X3D_INFO, "Alloc'd %d bytes\n", size);
  
  void* mem = malloc(size);

  if(!mem) {
    x3d_error(0, "Out of memory!");
  }

  return mem;
}

void x3d_free(X3D_Context* context, void* ptr) {
  free(ptr);
}

void* x3d_realloc(X3D_Context* context, void* ptr, uint16 size) {
  return realloc(ptr, size);
}

