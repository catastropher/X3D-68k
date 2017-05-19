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

#include "geo/X_Vec2.h"
#include "math/X_angle.h"

typedef struct X_Viewport
{
    X_Vec2 screenPos;
    int w;
    int h;
    int distToProjectionPlane;
    angle256 fieldOfView;
} X_Viewport;

void x_viewport_init(X_Viewport* viewport, X_Vec2 screenPos, int w, int h, angle256 fieldOfView);

