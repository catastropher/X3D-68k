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

#include "X_BspLevel.h"
#include "X_RayTracer.h"
#include "geo/X_Ray3.h"
#include "X_BspRayTracer.hpp"

typedef struct X_RayTracer
{
    BspLevel* level;
    Ray3 ray;
    int rootClipNode;
    Plane collisionPlane;
    Vec3 collisionPoint;
    x_fp16x16 collisionFrac;
    Vec3* modelOrigin;
    BspModel* hitModel;

    bool hitSomething;
    bool useBspTree;
} X_RayTracer;

template<typename IdType, typename NodeType>
void x_raytracer_from_bspraytracer(BspRayTracer<IdType, NodeType>& bspRayTracer, X_RayTracer& dest, BspLevel* level)
{
    auto& collision = bspRayTracer.getCollision();

    dest.hitModel = collision.hitModel;
    dest.collisionFrac = collision.location.t.toFp16x16();
    dest.collisionPlane = collision.plane;
    dest.collisionPoint = MakeVec3(collision.location.point);
    //dest.ray = tracerray;
    dest.level = level;
}

void x_raytracer_init(X_RayTracer* trace, BspLevel* level, BspModel* model, Vec3* start, Vec3* end, BoundBox* boundBox);
bool x_raytracer_trace(X_RayTracer* trace);

template<typename IdType, typename NodeType>
void x_raytracer_from_bspraytracer(BspRayTracer<IdType, NodeType>& bspRaytracer, X_RayTracer& dest, BspLevel* level);

