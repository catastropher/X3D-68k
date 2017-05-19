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

#include "X_Viewport.h"
#include "math/X_trig.h"

static inline int calculate_distance_to_projection_plane(int w, angle256 fieldOfView)
{
    return x_fp16x16_make(w / 2) / x_tan(fieldOfView / 2);
}

void x_viewport_init(X_Viewport* viewport, X_Vec2 screenPos, int w, int h, angle256 fieldOfView)
{
    viewport->screenPos = screenPos;
    viewport->w = w;
    viewport->h = h;
    viewport->distToProjectionPlane = calculate_distance_to_projection_plane(w, fieldOfView);
}

