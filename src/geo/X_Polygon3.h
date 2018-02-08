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

struct Polygon3
{
    Polygon3(X_Vec3* vertices_, int totalVertices_) :
        vertices(vertices_),
        totalVertices(totalVertices_) { }
        
    bool clipToPlane(const X_Plane& plane, Polygon3& dest) const;
    bool clipToPlanePreserveEdgeIds(const X_Plane& plane, Polygon3& dest, int* edgeIds, int* edgeIdsDest) const;
    void splitAlongPlane(const X_Plane& plane, int* edgeIds, Polygon3& frontSide, int* frontEdgeIds, Polygon3& backSide, int* backEdgeIds) const;
    void clone(Polygon3& dest) const;
    bool clipToFrustum(const X_Frustum& frustum, Polygon3& dest, unsigned int clipFlags) const;
    bool clipToFrustumPreserveEdgeIds(const X_Frustum& frustum, Polygon3& dest, unsigned int clipFlags, int* edgeIds, int* edgeIdsDest);
    void reverse();
    
    X_Vec3* vertices;
    int totalVertices;
};

class InternalPolygon3 : public Polygon3
{
public:
    InternalPolygon3(int totalVertices_) : Polygon3(internalVertices, totalVertices_) { }
    InternalPolygon3() : Polygon3(internalVertices, X_POLYGON3_MAX_VERTS) { }
    
private:
    X_Vec3 internalVertices[X_POLYGON3_MAX_VERTS];
};

void x_polygon3_render_wireframe(const Polygon3* poly, X_RenderContext* rcontext, X_Color color);
void x_polygon3_render_flat_shaded(Polygon3* poly, X_RenderContext* renderContext, X_Color color);
void x_polygon3_render_textured(Polygon3* poly, X_RenderContext* renderContext, X_Texture* texture, X_Vec2 textureCoords[3]);
void x_polygon3_render_transparent(Polygon3* poly, X_RenderContext* renderContext, X_Color* transparentTable);

