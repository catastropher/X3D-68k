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
#include "memory/X3D_list.h"
#include "X3D_assert.h"

///////////////////////////////////////////////////////////////////////////////
/// Initializes a list of uint16's.
///
/// @param list     - list
/// @param mem      - memory for the list
/// @param mem_size - size of mem
///
/// @return Nothing.
/// @note mem_size must be even.
///////////////////////////////////////////////////////////////////////////////
void x3d_list_uint16_init(X3D_List_uint16* list, void* mem, uint16 mem_size) {
  // Make sure we have an even memory size!
  x3d_assert((mem_size & 1) == 0);
  
  list->data = mem;
  list->capacity = mem_size / sizeof(uint16);
  list->size = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// Adds an element to the end of a list of uint16's.
///
/// @param list - list
/// @param val  - value to add
///
/// @return The index in the list the value was added to.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_list_uint16_add(X3D_List_uint16* list, uint16 val) {
  // Prevent list overflow
  x3d_assert(list->size < list->capacity);
  
  list->data[list->size] = val;
  
  return list->size++;
}

///////////////////////////////////////////////////////////////////////////////
/// Gets an element from a list.
///
/// @param list   - list
/// @param index  - index in the list
///
/// @param The element from the list.
///////////////////////////////////////////////////////////////////////////////
uint16 x3d_list_uint16_get(X3D_List_uint16* list, uint16 index) {
  // Check for out of bounds
  x3d_assert(index < list->capacity);
  
  return list->data[index];
}