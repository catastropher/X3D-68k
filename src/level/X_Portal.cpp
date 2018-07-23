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
#include "level/X_CollisionHullBuilder.hpp"

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
    bridgeModel.planes = bridgePlanes;
    bridgeModel.clipNodes = bridgeClipNodes;
    bridgeModel.origin = Vec3(0, 0, 0);

    CollisionHullBuilder collisionHullBuilder(bridgeModel, poly, orientation, fp::fromInt(100), fp::fromFloat(-2));
    collisionHullBuilder.build();

    x_link_init(&bridgeModel.objectsOnModelHead, &bridgeModel.objectsOnModelTail);
}

