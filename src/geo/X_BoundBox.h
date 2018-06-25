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

#include <algorithm>

#include "X_Vec3.h"
#include "util/X_util.h"

struct Plane;
struct X_Frustum;

typedef enum BoundBoxPlaneFlags
{
    X_BOUNDBOX_OUTSIDE_PLANE = 0,
    X_BOUNDBOX_INSIDE_PLANE = 1,
    X_BOUNDBOX_INTERSECT_PLANE = 2
} BoundBoxPlaneFlags;

typedef enum BoundBoxFrustumFlags
{
    X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM = -1,
    X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM = 0,
} BoundBoxFrustumFlags;

struct BoundBox
{
    BoundBox()
    {
        v[0].x = 0x7FFFFFFF;
        v[0].y = 0x7FFFFFFF;
        v[0].z = 0x7FFFFFFF;

        v[1].x = -0x7FFFFFFF;
        v[1].y = -0x7FFFFFFF;
        v[1].z = -0x7FFFFFFF;
    }

    void addPoint(const Vec3& point)
    {
        v[0].x = std::min(v[0].x, point.x);
        v[0].y = std::min(v[0].y, point.y);
        v[0].z = std::min(v[0].z, point.z);
        
        v[1].x = std::max(v[1].x, point.x);
        v[1].y = std::max(v[1].y, point.y);
        v[1].z = std::max(v[1].z, point.z);
    }

    void merge(const BoundBox& box, BoundBox& dest) const
    {
        dest.v[0].x = std::min(v[0].x, box.v[0].x);
        dest.v[0].y = std::min(v[0].y, box.v[0].y);
        dest.v[0].z = std::min(v[0].z, box.v[0].z);
        
        dest.v[1].x = std::max(v[1].x, box.v[1].x);
        dest.v[1].y = std::max(v[1].y, box.v[1].y);
        dest.v[1].z = std::max(v[1].z, box.v[1].z);
    }

    BoundBoxPlaneFlags determinePlaneClipFlags(const Plane& plane) const;
    BoundBoxFrustumFlags determineFrustumClipFlags(const X_Frustum& frustum, BoundBoxFrustumFlags parentFlags) const;

    void print() const;

    static bool clipAgainstFrustumPlane(BoundBoxFrustumFlags flags, int planeId)
    {
        return flags & (1 << planeId);
    }

    Vec3 v[2];
};

