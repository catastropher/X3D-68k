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

typedef uint16 X3D_Handle;

typedef struct X3D_HandleManager {
  uint16 total_h;
  void** handles;
  void** head;
  void** tail;
} X3D_HandleManager;

X3D_Handle x3d_handle_add(void* ptr);
void x3d_handle_delete(X3D_Handle handle);
void* x3d_handle_deref(X3D_Handle handle);
void x3d_handlemanager_init(uint16 total_h);

