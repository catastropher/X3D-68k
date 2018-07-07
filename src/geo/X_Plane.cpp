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

Plane::Plane(const Vec3fp& a, const Vec3fp& b, const Vec3fp& c)
{
    Vec3fp v1 = a - b;
    Vec3fp v2 = c - b;

    v1.normalize();
    v2.normalize();

    normal = v1.cross(v2);

    d = -normal.dot(a);
}

void Plane::print() const
{
    float x = normal.x.toFloat();
    float y = normal.y.toFloat();
    float z = normal.z.toFloat();
    float dd = d.toFloat();
    
    printf("%fX + %fY + %fZ + %f = 0\n", x, y, z, dd);
}

void Plane::getOrientation(X_CameraObject& cam, Mat4x4& dest) const
{
    Vec3fp temp(normal.x, 0, normal.z);
    Vec3fp camPos = MakeVec3fp(cam.collider.position);
    
    Mat4x4 mat;
    mat.loadYRotation(X_ANG_270);
    
    Vec3fp right, up;
    
    if(abs(normal.y) != fp::fromInt(1))
    {
        right = mat.transform(temp);
        right.normalize();
        up = normal.cross(right);
    }
    else
    {
        // Pick the vectors from the cam direction
        Vec3fp temp;
        cam.viewMatrix.extractViewVectors(up, right, temp);
        
        right.y = 0;
        right.normalize();
        
        up.y = 0;
        up.normalize();

        // On the ceiling, so we need to reverse the up direction
        if(normal.y == fp::fromInt(1))
        {
            up = -up;
        }
    }
    
    dest.loadIdentity();
    
    Vec4 up4 = Vec4(up);
    Vec4 right4 = Vec4(right);

    Vec4 forward4 = Vec4(normal);
    
    dest.setColumn(0, right4);
    dest.setColumn(1, up4);
    dest.setColumn(2, forward4);
}

