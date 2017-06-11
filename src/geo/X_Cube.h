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

#include "geo/X_Vec3.h"
#include "object/X_CameraObject.h"
#include "render/X_Canvas.h"
#include "math/X_Mat4x4.h"

typedef struct X_Cube
{
    X_Vec3 vertices[8];
} X_Cube;

void x_cube_init(X_Cube* cube, int width, int height, int depth);
void x_cube_translate(X_Cube* cube, X_Vec3 translation);
void x_cube_render(const X_Cube* cube, const X_CameraObject* cam, X_Canvas* canvas, X_Color color);
void x_cube_transform(const X_Cube* src, X_Cube* dest, const X_Mat4x4* mat);

