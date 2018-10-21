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

#include "X_CollisionHullBuilder.hpp"
#include "math/X_Mat4x4.h"
#include "geo/X_Polygon3.h"
#include "geo/X_Plane.h"
#include "level/X_BspLevel.h"

void CollisionHullBuilder::build()
{
    center = polygon.calculateCenter();
    calculateAxisFromOrientation();

    for(int i = 0; i < 1; ++i)
    {
        buildClipNodeHull(i);
    }
}

void CollisionHullBuilder::calculateAxisFromOrientation()
{
    Vec3fp forward, right, up;
    orientation.extractViewVectors(forward, right, up);

    axis[0] = right;
    axis[1] = up;
    axis[2] = -right;
    axis[3] = -up;
    axis[4] = -forward;
    axis[5] = forward;
}

void CollisionHullBuilder::buildClipNodeHull(int level)
{
    model.clipNodeRoots[level] = buildHullSidesRecursively(0);
}

int CollisionHullBuilder::buildHullSidesRecursively(int depth)
{
    if(depth == 6)
    {
        return X_BSPLEAF_REGULAR;
    }

    int nodeId = allocateClipNode();
    X_BspClipNode& node = model.clipNodes[nodeId];

    node.frontChild = buildHullSidesRecursively(depth + 1);
    node.backChild = X_BSPLEAF_SOLID;
    node.planeId = createCollisionPlane(depth);

    return nodeId;
}

Vec3fp CollisionHullBuilder::findPointOnPlane(int axisId)
{
    const int FRONT_PLANE = 4;
    const int BACK_PLANE = 5;

    if(axisId == FRONT_PLANE)
    {
        return center - axis[axisId] * frontDepth;
    }
    else if(axisId == BACK_PLANE)
    {
        return center - axis[axisId] * backDepth;
    }
    else
    {
        return vertexWithLargestProjection(-axis[axisId]);
    }
}

int CollisionHullBuilder::createCollisionPlane(int axisId)
{
    Vec3fp pointOnPlane = findPointOnPlane(axisId);

    int planeId = allocatePlane();
    model.planes[planeId].plane = Plane(axis[axisId], pointOnPlane);

    return planeId;
}

Vec3fp& CollisionHullBuilder::vertexWithLargestProjection(const Vec3fp& axis)
{
    int largestIndex = 0;
    fp largestProjection = minValue<fp>();

    for(int i = 0; i < polygon.totalVertices; ++i)
    {
        fp dot = (polygon.vertices[i] - center).dot(axis);

        if(dot > largestProjection)
        {
            largestProjection = dot;
            largestIndex = i;
        }
    }

    return polygon.vertices[largestIndex];
}

