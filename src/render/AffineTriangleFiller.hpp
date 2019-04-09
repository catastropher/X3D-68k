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

#include "RenderContext.hpp"
#include "Texture.hpp"


typedef enum X_TriangleFillerType
{
    X_TRIANGLE_FLAT = 0,
    X_TRIANGLE_GENERIC = 1
} X_TriangleFillerType;

typedef struct X_TriangleFillerVertex
{
    Vec2i v;
    fp z;

    Vec2i textureCoord;
} X_TriangleFillerVertex;

typedef struct X_TriangleFillerEdge
{
    fp x;
    fp xSlope;

    fp z;
    fp zSlope;

    fp u;
    fp uSlope;

    fp v;
    fp vSlope;

    int endY;
} X_TriangleFillerEdge;

typedef struct X_TriangleFiller
{
    X_TriangleFillerVertex vertices[3];
    X_TriangleFillerType type;

    X_Color fillColor;
    Texture* fillTexture;
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
void x_trianglefiller_fill_textured(X_TriangleFiller* filler, Texture* texture);
void x_trianglefiller_fill_transparent(X_TriangleFiller* filler, X_Color* transparentTable);

void x_trianglefiller_init(X_TriangleFiller* filler, X_RenderContext* renderContext);

#define X_TRIANGLEFILLER_EXTRA_PRECISION 4

static inline void x_trianglefiller_set_flat_shaded_vertex(X_TriangleFiller* filler, int vertexId, Vec2i vertex, int z)
{
    filler->vertices[vertexId].v = vertex;
    filler->vertices[vertexId].z = (1.0_fp << X_TRIANGLEFILLER_EXTRA_PRECISION) / z;
}

static inline void x_trianglefiller_set_textured_vertex(X_TriangleFiller* filler, int vertexId, Vec2i vertex, int z, Vec2i textureCoord)
{
    filler->vertices[vertexId].v = vertex;
    filler->vertices[vertexId].z = (1.0_fp << X_TRIANGLEFILLER_EXTRA_PRECISION) / z;
    filler->vertices[vertexId].textureCoord = textureCoord;
}

// TODO: find a better place for this
struct ModelVertex
{
    Vec3fp v;
    int x;
    int y;
    int s;
    int t;
    int z;

    void print(const char* name) const
    {
        printf("%s: x=%d, y=%d, z=%d, s=%d, t=%d\n", name, x, y, z, s, t);
    }
};

// TODO: find a better place for this
void x_polygon3_render_textured(ModelVertex* vertices, int totalVertices, X_RenderContext* renderContext, Texture* texture);
