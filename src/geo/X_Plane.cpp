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
#include "object/X_CameraObject.h"

void x_plane_init_from_three_points(X_Plane* plane, const Vec3* a, const Vec3* b, const Vec3* c)
{
    Vec3 v1 = x_vec3_sub(a, b);
    Vec3 v2 = x_vec3_sub(c, b);

    x_vec3_normalize(&v1);
    x_vec3_normalize(&v2);

    plane->normal = MakeVec3fp(x_vec3_cross(&v1, &v2));

    Vec3fp temp = MakeVec3fp(*a);
    plane->d = -plane->normal.dot(temp);
}

void x_plane_print(const X_Plane* plane)
{
    float x = plane->normal.x.toFloat();
    float y = plane->normal.y.toFloat();
    float z = plane->normal.z.toFloat();
    float d = plane->d.toFloat();
    
    printf("%fX + %fY + %fZ + %f = 0\n", x, y, z, d);
}

void x_plane_get_orientation(X_Plane* plane, X_CameraObject* cam, X_Mat4x4* dest)
{
    Vec3 temp = MakeVec3(plane->normal);
    temp.y = 0;
    
    X_Mat4x4 mat;
    x_mat4x4_load_y_rotation(&mat, X_ANG_270);
    
    Vec3 right, up;
    
    if(abs(plane->normal.y) != X_FP16x16_ONE)
    {
        x_mat4x4_transform_vec3(&mat, &temp, &right);
        x_vec3_normalize(&right);
        
        Vec3 temp = MakeVec3(plane->normal);

        up = x_vec3_cross(&temp, &right);
    }
    else
    {
        // Pick the vectors from the cam direction
        Vec3 temp;
        x_mat4x4_extract_view_vectors(&cam->viewMatrix, &up, &right, &temp);
        
        right.y = 0;
        x_vec3_normalize(&right);
        
        up.y = 0;
        x_vec3_normalize(&up);
    }
    
    x_mat4x4_load_identity(dest);
    
    X_Vec4 up4 = x_vec4_from_vec3(&up);
    X_Vec4 right4 = x_vec4_from_vec3(&right);

    temp = MakeVec3(plane->normal);

    X_Vec4 forward4 = x_vec4_from_vec3(&temp);
    
    x_mat4x4_set_column(dest, 0, &right4);
    x_mat4x4_set_column(dest, 1, &up4);
    x_mat4x4_set_column(dest, 2, &forward4);
}

