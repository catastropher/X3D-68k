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

#define X_POLYGON3_MAX_VERTS 100

typedef struct X_Polygon3
{
    int totalVertices;
    X_Vec3_fp16x16* vertices;
} X_Polygon3;

_Bool x_polygon3_fp16x16_clip_to_plane(const X_Polygon3* src, const X_Plane* plane, X_Polygon3* dest);

void x_polygon3_render_wireframe(const X_Polygon3* poly, X_RenderContext* rcontext, X_Color color);
void x_polygon3_render_flat_shaded(X_Polygon3* poly, X_RenderContext* renderContext, X_Color color);
void x_polygon3_render_textured(X_Polygon3* poly, X_RenderContext* renderContext, X_Texture* texture, X_Vec2 textureCoords[3]);
void x_polygon3_render_transparent(X_Polygon3* poly, X_RenderContext* renderContext, X_Color* transparentTable);

void x_polygon3d_copy(const X_Polygon3* src, X_Polygon3* dest);

_Bool x_polygon3_fp16x16_clip_to_frustum(const X_Polygon3* poly, const X_Frustum* frustum, X_Polygon3* dest, unsigned int clipFlags);
_Bool x_polygon3_fp16x16_clip_to_frustum_edge_ids(const X_Polygon3* poly, const X_Frustum* frustum, X_Polygon3* dest,
                                                  unsigned int clipFlags, int* edgeIds, int* edgeIdsDest);

static inline X_Polygon3 x_polygon3_make(X_Vec3* vertices, int totalVertices)
{
    return (X_Polygon3) { totalVertices, vertices };
}

