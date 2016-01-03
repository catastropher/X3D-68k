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

#pragma once

#include "X3D_common.h"

///////////////////////////////////////////////////////////////////////////////
/// A list of uint16's (though technically not a list).
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_List_uint16 {
  uint16 capacity;      ///< The number of elements the list can hold
  uint16 size;          ///< The number of elements the list currently has
  uint16* data;         ///< Data
} X3D_List_uint16;

void x3d_list_uint16_init(X3D_List_uint16* list, void* mem, uint16 mem_size);
uint16 x3d_list_uint16_add(X3D_List_uint16* list, uint16 val);
uint16 x3d_list_uint16_get(X3D_List_uint16* list, uint16 index);

