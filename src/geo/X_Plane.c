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

#include <stdio.h>

#include "X_Plane.h"
#include "math/X_fix.h"

void x_plane_init_from_three_points(X_Plane* plane, const X_Vec3* a, const X_Vec3* b, const X_Vec3* c)
{
    X_Vec3 v1 = x_vec3_sub(a, b);
    X_Vec3 v2 = x_vec3_sub(c, b);

    x_vec3_fp16x16_normalize(&v1);
    x_vec3_fp16x16_normalize(&v2);
    
    plane->normal = x_vec3_fp16x16_cross(&v1, &v2);
    plane->d = -x_vec3_fp16x16_dot(&plane->normal, a);
}

void x_plane_print(const X_Plane* plane)
{
    float x = x_fp16x16_to_float(plane->normal.x);
    float y = x_fp16x16_to_float(plane->normal.y);
    float z = x_fp16x16_to_float(plane->normal.z);
    float d = x_fp16x16_to_float(plane->d);
    
    printf("%fX + %fY + %fZ + %f = 0\n", x, y, z, d);
}

