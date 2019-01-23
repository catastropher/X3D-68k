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

#include "geo/Vec2.hpp"
#include "Texture.hpp"
#include "math/FixedPoint.hpp"
#include "RenderContext.hpp"

typedef enum X_TriangleFillerType
{
    X_TRIANGLE_FLAT = 0,
    X_TRIANGLE_GENERIC = 1
} X_TriangleFillerType;

typedef struct X_TriangleFillerVertex
{
    X_Vec2 v;
    x_fp16x16 z;
    
    X_Vec2 textureCoord;
} X_TriangleFillerVertex;

typedef struct X_TriangleFillerEdge
{
    x_fp16x16 x;
    x_fp16x16 xSlope;
    
    x_fp16x16 z;
    x_fp16x16 zSlope;
    
    x_fp16x16 u;
    x_fp16x16 uSlope;
    
    x_fp16x16 v;
    x_fp16x16 vSlope;
    
    int endY;
} X_TriangleFillerEdge;

typedef struct X_TriangleFiller
{
    X_TriangleFillerVertex vertices[3];
    X_TriangleFillerType type;
    
    X_Color fillColor;
    X_Texture* fillTexture;
    X_Color* transparentTable;
    
    int y;
    int endY;
    
    X_TriangleFillerEdge edges[3];
    int nextEdge;
    
    X_TriangleFillerEdge* leftEdge;
    X_TriangleFillerEdge* rightEdge;
    X_TriangleFillerEdge** firstEndingEdge;
    
    X_TriangleFillerEdge* middleStartingEdge;
    
    X_RenderContext* renderContext;
    
    void (*drawSpan)(struct X_TriangleFiller*);
} X_TriangleFiller;

void x_trianglefiller_fill_flat_shaded(X_TriangleFiller* filler, X_Color color);
void x_trianglefiller_fill_textured(X_TriangleFiller* filler, X_Texture* texture);
void x_trianglefiller_fill_transparent(X_TriangleFiller* filler, X_Color* transparentTable);

void x_trianglefiller_init(X_TriangleFiller* filler, X_RenderContext* renderContext);

#define X_TRIANGLEFILLER_EXTRA_PRECISION 4

static inline void x_trianglefiller_set_flat_shaded_vertex(X_TriangleFiller* filler, int vertexId, X_Vec2 vertex, int z)
{
    filler->vertices[vertexId].v = vertex;
    filler->vertices[vertexId].z = (X_FP16x16_ONE << X_TRIANGLEFILLER_EXTRA_PRECISION) / z;
}

static inline void x_trianglefiller_set_textured_vertex(X_TriangleFiller* filler, int vertexId, X_Vec2 vertex, int z, X_Vec2 textureCoord)
{
    filler->vertices[vertexId].v = vertex;
    filler->vertices[vertexId].z = (X_FP16x16_ONE << X_TRIANGLEFILLER_EXTRA_PRECISION) / z;
    filler->vertices[vertexId].textureCoord = textureCoord;
}

