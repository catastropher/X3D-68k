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
#include "util/X_util.h"

typedef enum X_BoundBoxPlaneFlags
{
    X_BOUNDBOX_OUTSIDE_PLANE = 0,
    X_BOUNDBOX_INSIDE_PLANE = 1,
    X_BOUNDBOX_INTERSECT_PLANE = 2
} X_BoundBoxPlaneFlags;

typedef enum X_BoundBoxFrustumFlags
{
    X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM = -1,
    X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM = 0,
} X_BoundBoxFrustumFlags;

typedef struct X_BoundBox
{
    X_Vec3_fp16x16 v[2];
} X_BoundBox;

struct X_Plane;
struct X_Frustum;

X_BoundBoxPlaneFlags x_boundbox_determine_plane_clip_flags(X_BoundBox* box, struct X_Plane* plane);
X_BoundBoxFrustumFlags x_boundbox_determine_frustum_clip_flags(X_BoundBox* box, struct X_Frustum* frustum, X_BoundBoxFrustumFlags parentFlags);
void x_boundbox_print(X_BoundBox* box);

static inline _Bool x_boundbox_clip_against_frustum_plane(X_BoundBoxFrustumFlags flags, int planeId)
{
    return flags & (1 << planeId);
}

static inline void x_boundbox_init(X_BoundBox* box)
{
    box->v[0].x = 0x7FFFFFFF;
    box->v[0].y = 0x7FFFFFFF;
    box->v[0].z = 0x7FFFFFFF;
    
    box->v[1].x = -0x7FFFFFFF;
    box->v[1].y = -0x7FFFFFFF;
    box->v[1].z = -0x7FFFFFFF;
}

static inline void x_boundbox_add_point(X_BoundBox* box, X_Vec3_fp16x16 point)
{
    box->v[0].x = X_MIN(box->v[0].x, point.x);
    box->v[0].y = X_MIN(box->v[0].y, point.y);
    box->v[0].z = X_MIN(box->v[0].z, point.z);
    
    box->v[1].x = X_MAX(box->v[1].x, point.x);
    box->v[1].y = X_MAX(box->v[1].y, point.y);
    box->v[1].z = X_MAX(box->v[1].z, point.z);
}

static inline void x_boundbox_merge(X_BoundBox* a, X_BoundBox* b, X_BoundBox* dest)
{
    dest->v[0].x = X_MIN(a->v[0].x, b->v[0].x);
    dest->v[0].y = X_MIN(a->v[0].y, b->v[0].y);
    dest->v[0].z = X_MIN(a->v[0].z, b->v[0].z);
    
    dest->v[1].x = X_MAX(a->v[1].x, b->v[1].x);
    dest->v[1].y = X_MAX(a->v[1].y, b->v[1].y);
    dest->v[1].z = X_MAX(a->v[1].z, b->v[1].z);
}

