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

typedef struct X3D_BoundRect {
    X3D_Vex2D v[2];
} X3D_BoundRect;

static inline void x3d_boundrect_calculate_from_points(X3D_BoundRect* rect, X3D_Vex2D v[], int total_v) {
    rect->v[0] = v[0];
    rect->v[1] = v[0];
    
    for(int i = 1; i < total_v; ++i) {
        rect->v[0].x = X3D_MIN(rect->v[0].x, v[i].x);
        rect->v[0].y = X3D_MIN(rect->v[0].y, v[i].y);
        
        rect->v[1].x = X3D_MAX(rect->v[1].x, v[i].x);
        rect->v[1].y = X3D_MAX(rect->v[1].y, v[i].y);
    }
}

