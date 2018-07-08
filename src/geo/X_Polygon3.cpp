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

#include <algorithm>

#include "X_Polygon3.h"
#include "X_Plane.h"
#include "X_Ray3.h"
#include "render/X_TriangleFiller.h"
#include "X_Frustum.h"
#include "math/X_Mat4x4.h"

bool Polygon3::clipToPlane(const Plane& plane, Polygon3& dest) const
{
    dest.totalVertices = 0;
    
    x_fp16x16 dot = plane.normal.dot(vertices[0]).toFp16x16();
    bool in = dot >= -plane.d;
    
    for(int i = 0; i < totalVertices; ++i)
    {
        int next = (i + 1 < totalVertices ? i + 1 : 0);
        
        if(in)
            dest.vertices[dest.totalVertices++] = vertices[i];
        
        x_fp16x16 nextDot = plane.normal.dot(vertices[next]).toFp16x16();
        bool nextIn = nextDot >= -plane.d;
        int dotDiff = nextDot - dot;
        
        if(in != nextIn && dotDiff != 0)
        {
            x_fp16x16 scale = x_fp16x16_div(-plane.d.toFp16x16() - dot, dotDiff);
            Ray3 ray(vertices[i], vertices[next]);
            dest.vertices[dest.totalVertices] = ray.lerp(fp(scale));
            
            ++dest.totalVertices;
        }
        
        dot = nextDot;
        in = nextIn;
    }
    
    return dest.totalVertices > 2;
}

bool Polygon3::clipToPlanePreserveEdgeIds(const Plane& plane, Polygon3& dest, int* edgeIds, int* edgeIdsDest) const
{
    dest.totalVertices = 0;
    
    x_fp16x16 dot = plane.normal.dot(vertices[0]).toFp16x16();
    bool in = dot >= -plane.d;
    
    for(int i = 0; i < totalVertices; ++i)
    {
        int next = (i + 1 < totalVertices ? i + 1 : 0);
        
        if(in)
        {
            dest.vertices[dest.totalVertices++] = vertices[i];
            *edgeIdsDest++ = edgeIds[i];
        }
        
        x_fp16x16 nextDot = plane.normal.dot(vertices[next]).toFp16x16();
        bool nextIn = nextDot >= -plane.d;
        int dotDiff = nextDot - dot;
        
        if(in != nextIn && dotDiff != 0)
        {
            x_fp16x16 scale = x_fp16x16_div(-plane.d.toFp16x16() - dot, dotDiff);
            Ray3 ray(vertices[i], vertices[next]);
            dest.vertices[dest.totalVertices] = ray.lerp(fp(scale));
            
            if(in)
                *edgeIdsDest++ = 0;    // Create a new edge
            else
                *edgeIdsDest++ = edgeIds[i];
            
            ++dest.totalVertices;
        }
        
        dot = nextDot;
        in = nextIn;        
    }
    
    return dest.totalVertices > 2;
}

void Polygon3::splitAlongPlane(const Plane& plane, int* edgeIds, Polygon3& frontSide, int* frontEdgeIds, Polygon3& backSide, int* backEdgeIds) const
{
    frontSide.totalVertices = 0;
    backSide.totalVertices = 0;
    
    fp dot = plane.normal.dot(vertices[0]);
    bool in = dot >= -plane.d;
    
    for(int i = 0; i < totalVertices; ++i)
    {
        int next = (i + 1 < totalVertices ? i + 1 : 0);
        
        if(in)
        {
            frontSide.vertices[frontSide.totalVertices++] = vertices[i];
            *frontEdgeIds++ = 0;//edgeIds[i];
        }
        else
        {
            backSide.vertices[backSide.totalVertices++] = vertices[i];
            *backEdgeIds++ = 0;//edgeIds[i];
        }
        
        fp nextDot = plane.normal.dot(vertices[next]);
        bool nextIn = nextDot >= -plane.d;
        fp dotDiff = nextDot - dot;
        
        if(in != nextIn && dotDiff != 0)
        {
            fp scale = (-plane.d - dot) / dotDiff;
            Ray3 ray(vertices[i], vertices[next]);
            
            frontSide.vertices[frontSide.totalVertices] = ray.lerp(scale);
            backSide.vertices[backSide.totalVertices] = frontSide.vertices[frontSide.totalVertices];
            
            
            if(in)
            {
                *frontEdgeIds++ = 0;    // Create a new edge
                *backEdgeIds++ = 0;
            }
            else
            {
                *frontEdgeIds++ = 0;// edgeIds[i];
                *backEdgeIds++ = 0;//edgeIds[i];
            }
            
            ++frontSide.totalVertices;
            ++backSide.totalVertices;
        }
        
        dot = nextDot;
        in = nextIn;        
    }    
}

Vec3fp Polygon3::calculateCenter() const
{
    Vec3fp center = vertices[0];

    for(int i = 1; i < totalVertices; ++i)
    {
        center += vertices[i];
    }

    return center / totalVertices;
}

void Polygon3::scaleRelativeToCenter(fp scale, Polygon3& dest) const
{
    Vec3fp center = calculateCenter();

    for(int i = 0; i < totalVertices; ++i)
    {
        dest.vertices[i] = (vertices[i] - center) * scale + center;
    }

    dest.totalVertices = totalVertices;
}

void x_polygon3_render_wireframe(const Polygon3* poly, X_RenderContext* rcontext, X_Color color)
{
    // for(int i = 0; i < poly->totalVertices; ++i)
    // {
    //     int next = (i + 1 < poly->totalVertices ? i + 1 : 0);
    //     Ray3 ray = x_ray3_make(poly->vertices[i], poly->vertices[next]);
        
    //     x_ray3_render(&ray, rcontext, color);
    // }
}

void Polygon3::clone(Polygon3& dest) const
{
    if(&dest == this)
        return;
    
    dest.totalVertices = totalVertices;
    memcpy(dest.vertices, vertices, totalVertices * sizeof(Vec3fp));
}

bool Polygon3::clipToFrustum(const X_Frustum& frustum, Polygon3& dest, unsigned int clipFlags) const
{
    InternalPolygon3 temp[2];
    
    int currentTemp = 0;
    const Polygon3* polyToClip = this;
    
    int lastClipPlane = 31 - __builtin_clz(clipFlags);
    
    for(int plane = 0; plane < lastClipPlane; ++plane)
    {
        bool clipToPlane = (clipFlags & (1 << plane)) != 0;
        if(!clipToPlane)
            continue;
        
        if(!polyToClip->clipToPlane(frustum.planes[plane], temp[currentTemp]))
            return 0;
        
        polyToClip = &temp[currentTemp];
        currentTemp ^= 1;
    }
    
    return polyToClip->clipToPlane(frustum.planes[lastClipPlane], dest);
}

// Clips a polygon to a frustum and keeps track of the id's of the edges
// If a new edge is inserted, it will have an ID of 0
bool Polygon3::clipToFrustumPreserveEdgeIds(const X_Frustum& frustum, Polygon3& dest, unsigned int clipFlags, int* edgeIds, int* edgeIdsDest)
{
    InternalPolygon3 tempPoly[2];
    
    int tempIds[2][100];
    
    int currentTemp = 0;
    const Polygon3* polyToClip = this;
    int* polyEdgeIds = edgeIds;
    
    int lastClipPlane = 31 - __builtin_clz(clipFlags);
    
    for(int plane = 0; plane < lastClipPlane; ++plane)
    {
        bool clipToPlane = (clipFlags & (1 << plane)) != 0;
        if(!clipToPlane)
            continue;
        
        if(!polyToClip->clipToPlanePreserveEdgeIds(frustum.planes[plane], tempPoly[currentTemp], polyEdgeIds, tempIds[currentTemp]))
            return 0;
        
        polyToClip = &tempPoly[currentTemp];
        polyEdgeIds = tempIds[currentTemp];
        currentTemp ^= 1;
    }
    
    return polyToClip->clipToPlanePreserveEdgeIds(frustum.planes[lastClipPlane], dest, polyEdgeIds, edgeIdsDest);
}

void Polygon3::reverse()
{
    for(int i = 0; i < totalVertices / 2; ++i)
        std::swap(vertices[i], vertices[totalVertices - i - 1]);
}

void Polygon3::constructRegular(int totalSides, fp sideLength, fp angleOffset, Vec3fp translation)
{
    fp angle = angleOffset;
    fp dAngle = fp::fromInt(256) / totalSides;

    const fp conversionFactor = fp(92681);          // 2 / sqrt(2)
    fp radius = sideLength * conversionFactor;

    for(int i = 0; i < totalSides; ++i)
    {
        vertices[i].x = radius * x_cos(angle) + translation.x;
        vertices[i].y = radius * x_sin(angle) + translation.y;
        vertices[i].z = translation.z;

        angle += dAngle;
    }

    totalVertices = totalSides;
}

void Polygon3::renderWireframe(X_RenderContext& renderContext, X_Color color)
{
    for(int i = 0; i < totalVertices; ++i)
    {
        int next = (i + 1 < totalVertices ? i + 1 : 0);

        Ray3 ray(vertices[i], vertices[next]);
        ray.render(renderContext, color);
    }
}

void x_polygon3_render_flat_shaded(Polygon3* poly, X_RenderContext* renderContext, X_Color color)
{
//     X_Vec3 clippedV[X_POLYGON3_MAX_VERTS];
//     X_Polygon3 clipped = x_polygon3_make(clippedV, X_POLYGON3_MAX_VERTS);
//    
//     // FIXME: this is broken
//     
// //     if(!x_polygon3_clip_to_frustum(poly, renderContext->viewFrustum, &clipped))
// //         return;
//     
//     if(clipped.totalVertices != 3)
//         return;
//     
//     X_TriangleFiller filler;
//     x_trianglefiller_init(&filler, renderContext);
//     
//     for(int i = 0; i < 3; ++i)
//     {
//         X_Vec3 transformed;
//         x_mat4x4_transform_vec3(renderContext->viewMatrix, clipped.vertices + i, &transformed);
//         
//         X_Vec2 projected;
//         x_viewport_project_vec3(&renderContext->cam->viewport, &transformed, &projected);
//         
//         x_trianglefiller_set_flat_shaded_vertex(&filler, i, projected, transformed.z);
//     }
//     
//     Plane plane;
//     //x_plane_init_from_three_points(&plane, poly->vertices + 0, poly->vertices + 1, poly->vertices + 2);
//     
//     //if(!x_plane_point_is_on_normal_facing_side(&plane, &renderContext->camPos))
//     //    return;
//     
//     x_trianglefiller_fill_flat_shaded(&filler, color);
}

void x_polygon3_render_textured(Polygon3* poly, X_RenderContext* renderContext, X_Texture* texture, X_Vec2 textureCoords[3])
{
//     X_Vec3 clippedV[X_POLYGON3_MAX_VERTS];
//     X_Polygon3 clipped = x_polygon3_make(clippedV, X_POLYGON3_MAX_VERTS);
//    
//     // FIXME: this is broken
// //     if(!x_polygon3_clip_to_frustum(poly, renderContext->viewFrustum, &clipped))
// //         return;
//     
//     if(clipped.totalVertices != 3)
//         return;
//     
//     X_TriangleFiller filler;
//     x_trianglefiller_init(&filler, renderContext);
//     
//     for(int i = 0; i < 3; ++i)
//     {
//         X_Vec3 transformed;
//         x_mat4x4_transform_vec3(renderContext->viewMatrix, clipped.vertices + i, &transformed);
//         
//         X_Vec2 projected;
//         x_viewport_project_vec3(&renderContext->cam->viewport, &transformed, &projected);
//         
//         x_trianglefiller_set_textured_vertex(&filler, i, projected, transformed.z, textureCoords[i]);
//     }
//     
// //     Plane plane;
// //     x_plane_init_from_three_points(&plane, poly->vertices + 0, poly->vertices + 1, poly->vertices + 2);
// //     
// //     if(!x_plane_point_is_on_normal_facing_side(&plane, &renderContext->camPos))
// //         return;
// //     
//     x_trianglefiller_fill_textured(&filler, texture);
}

void x_polygon3_render_transparent(Polygon3* poly, X_RenderContext* renderContext, X_Color* transparentTable)
{
//     X_Vec3 clippedV[X_POLYGON3_MAX_VERTS];
//     X_Polygon3 clipped = x_polygon3_make(clippedV, X_POLYGON3_MAX_VERTS);
//     
//     // FIXME this is broken
//     
// //     if(!x_polygon3_clip_to_frustum(poly, renderContext->viewFrustum, &clipped))
// //         return;
// //     
//     if(clipped.totalVertices != 3)
//         return;
//     
//     X_TriangleFiller filler;
//     x_trianglefiller_init(&filler, renderContext);
//     
//     for(int i = 0; i < 3; ++i)
//     {
//         X_Vec3 transformed;
//         x_mat4x4_transform_vec3(renderContext->viewMatrix, clipped.vertices + i, &transformed);
//         
//         X_Vec2 projected;
//         x_viewport_project_vec3(&renderContext->cam->viewport, &transformed, &projected);
//         
//         x_trianglefiller_set_flat_shaded_vertex(&filler, i, projected, transformed.z);
//     }
//     
//     Plane plane;
//     //x_plane_init_from_three_points(&plane, poly->vertices + 0, poly->vertices + 1, poly->vertices + 2);
//     
//    // if(!x_plane_point_is_on_normal_facing_side(&plane, &renderContext->camPos))
//    //     return;
//     
//     x_trianglefiller_fill_transparent(&filler, transparentTable);
}


