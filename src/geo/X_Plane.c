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

#include "X_Plane.h"

void x_plane_init_from_three_points(X_Plane* plane, const X_Vec3* a, const X_Vec3* b, const X_Vec3* c)
{
    X_Vec3 v1 = x_vec3_sub(a, b);
    X_Vec3 v2 = x_vec3_sub(c, b);
    
    plane->normal = x_vec3_cross(&v1, &v2);
    x_vec3_fp16x16_normalize(&plane->normal);
    
    x_plane_init_from_normal_and_point(plane, &plane->normal, a);
}

