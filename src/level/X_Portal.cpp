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

#include "X_Portal.hpp"

void Portal::linkTo(Portal* otherSide_)
{
    otherSide = otherSide_;

    if(otherSide == nullptr)
    {
        return;
    }

    Mat4x4 otherSideInverse = otherSide->orientation;
    otherSideInverse.transpose3x3();

    Mat4x4 rotate180AroundY;
    rotate180AroundY.loadYRotation(X_ANG_180);

    transformToOtherSide = orientation * rotate180AroundY * otherSideInverse;

    transformToOtherSide.extractEulerAngles(transformAngleX, transformAngleY);
}

Vec3fp Portal::transformPointToOtherSide(Vec3fp point)
{
    Vec3fp diff = point - center;

    Mat4x4 mat = transformToOtherSide;
    mat.dropTranslation();
    mat.transpose3x3();

    return mat.transform(diff) + otherSide->center;
}

Vec2fp Portal::projectPointOntoSurface(Vec3fp& point) const
{
    Vec3fp forward, right, up;
    orientation.extractViewVectors(forward, right, up);

    return Vec2fp(
        point.dot(right),
        point.dot(up));
}

void Portal::calculateSurfaceBoundRect()
{
    surfaceBoundRect.clear();

    for(int i = 0; i < poly.totalVertices; ++i)
    {
        Vec2fp pointOnSurface = projectPointOntoSurface(poly.vertices[i]);
        surfaceBoundRect.addPoint(pointOnSurface);
    }

    surfaceBoundRect.v[0].print();
    surfaceBoundRect.v[1].print();
}

bool Portal::pointInPortal(Vec3fp& point) const
{
    Vec2fp pointOnSurface = projectPointOntoSurface(point);
    return surfaceBoundRect.pointInside(pointOnSurface);
}

void Portal::updatePoly()
{
    center = poly.calculateCenter();
    calculateSurfaceBoundRect();

    // Create the bridge
    Vec3fp forward, up, right;
    orientation.extractViewVectors(forward, right, up);

    Vec3fp axis[4] = 
    { 
        right,
        up,
        -right,
        -up
    };

    for(int i = 0; i < 4; ++i)
    {
        Vec3fp& furthestPointAlongAxis = outlinePointWithLargestProjection(-axis[i]);

        bridgePlanes[i].plane = Plane(axis[i], furthestPointAlongAxis);
    }

    for(int i = 0; i < 4; ++i)
    {
        bridgeClipNodes[i].frontChild = i + 1;
        bridgeClipNodes[i].backChild = X_BSPLEAF_SOLID;
        bridgeClipNodes[i].planeId = i;
    }

    bridgeClipNodes[3].frontChild = X_BSPLEAF_REGULAR;

    bridgeModel.planes = bridgePlanes;
    bridgeModel.clipNodes = bridgeClipNodes;

    for(int i = 0; i < 3; ++i)
    {
        bridgeModel.clipNodeRoots[i] = 0;
    }
}

Vec3fp& Portal::outlinePointWithLargestProjection(const Vec3fp& axis)
{
    int largestIndex = 0;
    fp largestProjection = minValue<fp>();

    Vec3fp center = poly.calculateCenter();

    for(int i = 0; i < poly.totalVertices; ++i)
    {
        fp dot = (poly.vertices[i] - center).dot(axis);

        if(dot > largestProjection)
        {
            largestProjection = dot;
            largestIndex = i;
        }
    }

    return poly.vertices[largestIndex];
}

