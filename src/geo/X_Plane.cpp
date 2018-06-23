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

X_Plane::X_Plane(const Vec3fp& a, const Vec3fp& b, const Vec3fp& c)
{
    Vec3fp v1 = a - b;
    Vec3fp v2 = c - b;

    v1.normalize();
    v2.normalize();

    normal = v1.cross(v2);

    d = -normal.dot(a);
}

void X_Plane::print() const
{
    float x = normal.x.toFloat();
    float y = normal.y.toFloat();
    float z = normal.z.toFloat();
    float dd = d.toFloat();
    
    printf("%fX + %fY + %fZ + %f = 0\n", x, y, z, dd);
}

void X_Plane::getOrientation(X_CameraObject& cam, X_Mat4x4& dest) const
{
    Vec3 temp = MakeVec3(normal);
    temp.y = 0;
    
    X_Mat4x4 mat;
    mat.loadXRotation(X_ANG_270);
    
    Vec3 right, up;
    
    if(abs(normal.y) != X_FP16x16_ONE)
    {
        Vec3fp tempTemp = MakeVec3fp(temp);

        right = MakeVec3(mat.transform(tempTemp));
        x_vec3_normalize(&right);
        
        Vec3 temp = MakeVec3(normal);

        up = x_vec3_cross(&temp, &right);
    }
    else
    {
        // Pick the vectors from the cam direction
        Vec3 temp;
        x_mat4x4_extract_view_vectors(&cam.viewMatrix, &up, &right, &temp);
        
        right.y = 0;
        x_vec3_normalize(&right);
        
        up.y = 0;
        x_vec3_normalize(&up);
    }
    
    dest.loadIdentity();
    
    Vec416x16 up4 = Vec416x16(up);
    Vec416x16 right4 = Vec416x16(right);

    temp = MakeVec3(normal);

    Vec416x16 forward4 = Vec416x16(temp);
    
    dest.setColumn(0, right4);
    dest.setColumn(1, up4);
    dest.setColumn(2, forward4);
}

