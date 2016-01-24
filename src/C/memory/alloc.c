// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_common.h"
#include "memory/X3D_alloc.h"
#include "memory/X3D_slaballocator.h"
#include "X3D_enginestate.h"

/// @todo Document.

void x3d_allocmanager_init(size_t mem_size) {
  x3d_slaballocator_init(&x3d_allocmanager_get()->alloc, mem_size);
}

void* x3d_slab_alloc(uint16 size) {
  return x3d_slaballocator_alloc(&x3d_allocmanager_get()->alloc, size);
}

void x3d_slab_free(void* mem) {
  x3d_slaballocator_free(&x3d_allocmanager_get()->alloc, mem);
}

