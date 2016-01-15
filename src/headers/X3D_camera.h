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

#include "X3D_object.h"
#include "X3D_screen.h"


typedef struct X3D_CameraObject {
  X3D_DynamicObjectBase base;
  X3D_Vex3D shift;
  X3D_Vex3D pseduo_pos;
} X3D_CameraObject;

void x3d_camera_init(void);

