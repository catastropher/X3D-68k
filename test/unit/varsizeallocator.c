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

#include "X3D.h"

void test_varsizeallocator() {
  X3D_VarSizeAllocator alloc;
  
  x3d_varsizeallocator_init(&alloc, 2, 100);
  
  uint16 id;
  void* obj1 = x3d_varsizeallocator_alloc(&alloc, 1, &id);
  x3d_assert(id == 0);
  x3d_assert(obj1 == alloc.alloc_pool.base + alloc.alloc_pool.size - 2);
  x3d_assert(obj1 == x3d_varsizeallocator_get(&alloc, 0));
  
  void* obj2 = x3d_varsizeallocator_alloc(&alloc, 10, &id);
  x3d_assert(id == 1);
  x3d_assert(obj2 == alloc.alloc_pool.base + alloc.alloc_pool.size - 12);
  x3d_assert(obj2 == x3d_varsizeallocator_get(&alloc, 1));
  
}

