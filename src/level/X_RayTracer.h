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

typedef struct X_RayTracer
{
    X_BspLevel* level;
    X_Ray3_fp16x16 ray;
    int rootClipNode;
    X_Plane collisionPlane;
    Vec3 collisionPoint;
    x_fp16x16 collisionFrac;
    Vec3* modelOrigin;
    X_BspModel* hitModel;
} X_RayTracer;

void x_raytracer_init(X_RayTracer* trace, X_BspLevel* level, X_BspModel* model, Vec3* start, Vec3* end, X_BoundBox* boundBox);
bool x_raytracer_trace(X_RayTracer* trace);

