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

#include "X_Plane.h"

struct FrustumPlane : Plane
{
    FrustumPlane() { }

    FrustumPlane(const Vec3fp& a, const Vec3fp& b, const Vec3fp& c, int id_)
        : Plane(a, b, c),
        id(id_)
    {

    }

    FrustumPlane(const Vec3fp& normal_, const Vec3fp& point, int id_)
    {
        normal = normal_;
        d = -normal.dot(point);
        id = id_;
    }

    int id;
    FrustumPlane* next;
};

typedef struct X_Frustum
{
    FrustumPlane* planes;
    int totalPlanes;
} X_Frustum;

void x_frustum_print(const X_Frustum* frustum);

