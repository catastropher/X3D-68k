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

#include "Vec3.hpp"
#include "render/RenderContext.hpp"
#include "render/Screen.hpp"

struct Plane;

#define X_POLYGON3_MAX_VERTS 100

struct Polygon3
{
    Polygon3(Vec3* vertices_, int totalVertices_) :
        vertices((Vec3fp*)vertices_),
        totalVertices(totalVertices_) { }

    Polygon3(Vec3fp* vertices_, int totalVertices_) :
        vertices(vertices_),
        totalVertices(totalVertices_) { }
        
    bool clipToPlane(const Plane& plane, Polygon3& dest) const;
    bool clipToPlanePreserveEdgeIds(const Plane& plane, Polygon3& dest, int* edgeIds, int* edgeIdsDest) const;
    void splitAlongPlane(const Plane& plane, int* edgeIds, Polygon3& frontSide, int* frontEdgeIds, Polygon3& backSide, int* backEdgeIds) const;
    void clone(Polygon3& dest) const;
    bool clipToFrustum(const X_Frustum& frustum, Polygon3& dest, unsigned int clipFlags) const;
    bool clipToFrustumPreserveEdgeIds(const X_Frustum& frustum, Polygon3& dest, unsigned int clipFlags, int* edgeIds, int* edgeIdsDest);
    void reverse();

    void constructRegular(int totalSides, fp sideLength, fp angleOffset, Vec3fp translation);
    void renderWireframe(X_RenderContext& renderContext, X_Color color);

    Vec3fp calculateCenter() const;

    void scaleRelativeToCenter(fp scale, Polygon3& dest) const;
    void rotateRelateToCenter(Mat4x4& transform);
    void calculatePlaneEquation(Plane& dest);
    
    Vec3fp* vertices;
    int totalVertices;
};

class InternalPolygon3 : public Polygon3
{
public:
    InternalPolygon3(int totalVertices_) : Polygon3(internalVertices, totalVertices_) { }
    InternalPolygon3() : Polygon3(internalVertices, X_POLYGON3_MAX_VERTS) { }
    
private:
    Vec3 internalVertices[X_POLYGON3_MAX_VERTS];
};

struct LevelPolygon3 : Polygon3
{
    LevelPolygon3(Vec3* vertices_, int totalVertices_, int* edgeIds_)
    : Polygon3(vertices_, totalVertices_),
    edgeIds(edgeIds_) { }

    LevelPolygon3(Vec3fp* vertices_, int totalVertices_, int* edgeIds_)
    : Polygon3(vertices_, totalVertices_),
    edgeIds(edgeIds_) { } 

    int* edgeIds;
};

void x_polygon3_render_wireframe(const Polygon3* poly, X_RenderContext* rcontext, X_Color color);
void x_polygon3_render_flat_shaded(Polygon3* poly, X_RenderContext* renderContext, X_Color color);
void x_polygon3_render_textured(Polygon3* poly, X_RenderContext* renderContext, X_Texture* texture, Vec2 textureCoords[3]);
void x_polygon3_render_transparent(Polygon3* poly, X_RenderContext* renderContext, X_Color* transparentTable);

