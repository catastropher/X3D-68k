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
#include "render/X_RenderContext.h"

struct X_Plane;

typedef struct X_Polygon3
{
    int totalVertices;
    X_Vec3* vertices;
} X_Polygon3;

typedef X_Polygon3 X_Polygon3_fp16x16;

_Bool x_polygon3_clip_to_plane(const X_Polygon3* src, const struct X_Plane* plane, X_Polygon3* dest);
_Bool x_polygon3_fp16x16_clip_to_plane(const X_Polygon3_fp16x16* src, const X_Plane* plane, X_Polygon3_fp16x16* dest);

void x_polygon3_render_wireframe(const X_Polygon3* poly, X_RenderContext* rcontext, X_Color color);
void x_polygon3d_copy(const X_Polygon3* src, X_Polygon3* dest);

_Bool x_polygon3_clip_to_frustum(const X_Polygon3* poly, const X_Frustum* frustum, X_Polygon3* dest);
_Bool x_polygon3_fp16x16_clip_to_frustum(const X_Polygon3_fp16x16* poly, const X_Frustum* frustum, X_Polygon3_fp16x16* dest, unsigned int clipFlags);

void x_polygon3_to_polygon3_fp16x16(const X_Polygon3* poly, X_Polygon3_fp16x16* dest);
void x_polygon3_fp16x16_to_polygon3(const X_Polygon3_fp16x16* poly, X_Polygon3* dest);

void x_polygon3_reset_clip_counter();
int x_polygon3_get_clip_counter();

static inline X_Polygon3 x_polygon3_make(X_Vec3* vertices, int totalVertices)
{
    return (X_Polygon3) { totalVertices, vertices };
}

