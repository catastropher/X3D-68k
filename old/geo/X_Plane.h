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

#include "X_Vec3.h"
#include "math/X_fix.h"

struct Mat4x4;
struct X_CameraObject;

struct Plane
{
    Plane() { }

    Plane(const Vec3fp& a, const Vec3fp& b, const Vec3fp& c);

    Plane(const Vec3fp& normal_, const Vec3fp& point)
    {
        normal = normal_;
        d = -normal.dot(point);
    }

    fp distanceTo(const Vec3fp& v) const
    {
        return normal.dot(v) + d;
    }

    bool pointOnNormalFacingSide(const Vec3fp& v) const
    {
        return distanceTo(v) > 0;
    }

    void flip()
    {
        normal = -normal;
        d = -d;
    }

    void print() const;
    void getOrientation(X_CameraObject& cam, Mat4x4& dest) const;

    Vec3fp normal;
    fp d;
};

