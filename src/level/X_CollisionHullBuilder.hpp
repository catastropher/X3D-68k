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

#include "math/X_fix.h"
#include "geo/X_Vec3.h"

struct Polygon3;
struct BspModel;
struct Mat4x4;

class CollisionHullBuilder
{
public:
    CollisionHullBuilder(BspModel& model_, Polygon3& polygon_, Mat4x4& orientation_, fp frontDepth_, fp backDepth_)
        : model(model_),
        polygon(polygon_),
        orientation(orientation_),
        frontDepth(frontDepth_),
        backDepth(backDepth_),
        totalPlanes(0),
        totalClipNodes(0)
    {

    }

    void build();

private:
    Vec3fp& vertexWithLargestProjection(const Vec3fp& axis);

    void buildClipNodeHull(int level);
    void calculateAxisFromOrientation();

    int buildHullSidesRecursively(int depth);

    int createCollisionPlane(int axisId);
    Vec3fp findPointOnPlane(int axisId);

    int allocateClipNode()
    {
        return totalClipNodes++;
    }
    
    int allocatePlane()
    {
        return totalPlanes++;
    }

    BspModel& model;
    Polygon3& polygon;
    Mat4x4& orientation;
    fp frontDepth;
    fp backDepth;

    int totalPlanes;
    int totalClipNodes;

    Vec3fp axis[6];

    Vec3fp center;
};


