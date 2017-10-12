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

#include "X_Polygon3.h"
#include "X_Plane.h"
#include "X_Ray3.h"
#include "render/X_TriangleFiller.h"

_Bool x_polygon3_clip_to_plane(const X_Polygon3* src, const X_Plane* plane, X_Polygon3* dest)
{
    dest->totalVertices = 0;
    
    x_fp16x16 dot = x_vec3_dot(&plane->normal, src->vertices + 0);
    _Bool in = dot >= -plane->d;
    
    for(int i = 0; i < src->totalVertices; ++i)
    {
        int next = (i + 1 < src->totalVertices ? i + 1 : 0);
        
        if(in)
            dest->vertices[dest->totalVertices++] = src->vertices[i];
        
        x_fp16x16 nextDot = x_vec3_dot(&plane->normal, src->vertices + next);
        _Bool nextIn = nextDot >= -plane->d;
        int dotDiff = (nextDot - dot) >> 16;
        
        if(in != nextIn && dotDiff != 0)
        {
            x_fp16x16 scale = (-plane->d - dot) / dotDiff;
            X_Ray3 ray = x_ray3_make(src->vertices[i], src->vertices[next]);
            x_ray3_lerp(&ray, scale, dest->vertices + dest->totalVertices);
            
            ++dest->totalVertices;
        }
        
        dot = nextDot;
        in = nextIn;
    }
    
    return dest->totalVertices > 2;
}

_Bool x_polygon3_fp16x16_clip_to_plane(const X_Polygon3_fp16x16* src, const X_Plane* plane, X_Polygon3_fp16x16* dest)
{
    dest->totalVertices = 0;
    
    x_fp16x16 dot = x_vec3_fp16x16_dot(&plane->normal, src->vertices + 0);
    _Bool in = dot >= -plane->d;
    
    for(int i = 0; i < src->totalVertices; ++i)
    {
        int next = (i + 1 < src->totalVertices ? i + 1 : 0);
        
        if(in)
            dest->vertices[dest->totalVertices++] = src->vertices[i];
        
        x_fp16x16 nextDot = x_vec3_fp16x16_dot(&plane->normal, src->vertices + next);
        _Bool nextIn = nextDot >= -plane->d;
        int dotDiff = nextDot - dot;
        
        if(in != nextIn && dotDiff != 0)
        {
            x_fp16x16 scale = x_fp16x16_div(-plane->d - dot, dotDiff);
            X_Ray3_fp16x16 ray = x_ray3_make(src->vertices[i], src->vertices[next]);
            x_ray3_fp16x16_lerp(&ray, scale, dest->vertices + dest->totalVertices);
            
            ++dest->totalVertices;
        }
        
        dot = nextDot;
        in = nextIn;
    }
    
    return dest->totalVertices > 2;
}

_Bool x_polygon3_fp16x16_clip_to_plane_edge_ids(const X_Polygon3_fp16x16* src, const X_Plane* plane, X_Polygon3_fp16x16* dest, int* edgeIds, int* edgeIdsDest)
{
    dest->totalVertices = 0;
    
    x_fp16x16 dot = x_vec3_fp16x16_dot(&plane->normal, src->vertices + 0);
    _Bool in = dot >= -plane->d;
    
    for(int i = 0; i < src->totalVertices; ++i)
    {
        int next = (i + 1 < src->totalVertices ? i + 1 : 0);
        
        if(in)
        {
            dest->vertices[dest->totalVertices++] = src->vertices[i];
            *edgeIdsDest++ = edgeIds[i];
        }
        
        x_fp16x16 nextDot = x_vec3_fp16x16_dot(&plane->normal, src->vertices + next);
        _Bool nextIn = nextDot >= -plane->d;
        int dotDiff = nextDot - dot;
        
        if(in != nextIn && dotDiff != 0)
        {
            x_fp16x16 scale = x_fp16x16_div(-plane->d - dot, dotDiff);
            X_Ray3_fp16x16 ray = x_ray3_make(src->vertices[i], src->vertices[next]);
            x_ray3_fp16x16_lerp(&ray, scale, dest->vertices + dest->totalVertices);
            
            if(in)
                *edgeIdsDest++ = 0;    // Create a new edge
            else
                *edgeIdsDest++ = edgeIds[i];
            
            ++dest->totalVertices;
        }
        
        dot = nextDot;
        in = nextIn;        
    }
    
    return dest->totalVertices > 2;
}

void x_polygon3_render_wireframe(const X_Polygon3* poly, X_RenderContext* rcontext, X_Color color)
{
    for(int i = 0; i < poly->totalVertices; ++i)
    {
        int next = (i + 1 < poly->totalVertices ? i + 1 : 0);
        X_Ray3 ray = x_ray3_make(poly->vertices[i], poly->vertices[next]);
        
        x_ray3d_render(&ray, rcontext, color);
    }
}

void x_polygon3d_copy(const X_Polygon3* src, X_Polygon3* dest)
{
    if(dest == src)
        return;
    
    dest->totalVertices = src->totalVertices;
    memcpy(dest->vertices, src->vertices, src->totalVertices * sizeof(X_Vec3));
}

_Bool x_polygon3_clip_to_frustum(const X_Polygon3* poly, const X_Frustum* frustum, X_Polygon3* dest)
{
    X_Vec3 tempV[200];
    X_Polygon3 temp[2] = 
    {
        x_polygon3_make(tempV, 100),
        x_polygon3_make(tempV + 100, 100)
    };
    
    int currentTemp = 0;
    const X_Polygon3* polyToClip = poly;
    
    for(int i = 0; i < frustum->totalPlanes - 1; ++i)
    {
        if(!x_polygon3_clip_to_plane(polyToClip, frustum->planes + i, &temp[currentTemp]))
            return 0;
        
        polyToClip = &temp[currentTemp];
        currentTemp ^= 1;
    }
    
    return x_polygon3_clip_to_plane(polyToClip, frustum->planes + frustum->totalPlanes - 1, dest);
}

_Bool x_polygon3_fp16x16_clip_to_frustum(const X_Polygon3_fp16x16* poly, const X_Frustum* frustum, X_Polygon3_fp16x16* dest, unsigned int clipFlags)
{
    X_Vec3_fp16x16 tempV[200];
    X_Polygon3_fp16x16 temp[2] = 
    {
        x_polygon3_make(tempV, 100),
        x_polygon3_make(tempV + 100, 100)
    };
    
    int currentTemp = 0;
    const X_Polygon3_fp16x16* polyToClip = poly;
    
    int lastClipPlane = 31 - __builtin_clz(clipFlags);
    
    for(int plane = 0; plane < lastClipPlane; ++plane)
    {
        _Bool clipToPlane = (clipFlags & (1 << plane)) != 0;
        if(!clipToPlane)
            continue;
        
        if(!x_polygon3_fp16x16_clip_to_plane(polyToClip, frustum->planes + plane, &temp[currentTemp]))
            return 0;
        
        polyToClip = &temp[currentTemp];
        currentTemp ^= 1;
    }
    
    return x_polygon3_fp16x16_clip_to_plane(polyToClip, frustum->planes + lastClipPlane, dest);
}

// Clips a polygon to a frustum and keeps track of the id's of the edges
// If a new edge is inserted, it will have an ID of 0
_Bool x_polygon3_fp16x16_clip_to_frustum_edge_ids(const X_Polygon3_fp16x16* poly, const X_Frustum* frustum, X_Polygon3_fp16x16* dest,
                                                  unsigned int clipFlags, int* edgeIds, int* edgeIdsDest)
{
    X_Vec3_fp16x16 tempV[200];
    X_Polygon3_fp16x16 tempPoly[2] = 
    {
        x_polygon3_make(tempV, 100),
        x_polygon3_make(tempV + 100, 100)
    };
    int tempIds[2][100];
    
    int currentTemp = 0;
    const X_Polygon3_fp16x16* polyToClip = poly;
    int* polyEdgeIds = edgeIds;
    
    int lastClipPlane = 31 - __builtin_clz(clipFlags);
    
    for(int plane = 0; plane < lastClipPlane; ++plane)
    {
        _Bool clipToPlane = (clipFlags & (1 << plane)) != 0;
        if(!clipToPlane)
            continue;
        
        if(!x_polygon3_fp16x16_clip_to_plane_edge_ids(polyToClip, frustum->planes + plane, &tempPoly[currentTemp], polyEdgeIds, tempIds[currentTemp]))
            return 0;
        
        polyToClip = &tempPoly[currentTemp];
        polyEdgeIds = tempIds[currentTemp];
        currentTemp ^= 1;
    }
    
    return x_polygon3_fp16x16_clip_to_plane_edge_ids(polyToClip, frustum->planes + lastClipPlane, dest, polyEdgeIds, edgeIdsDest);
}

void x_polygon3_to_polygon3_fp16x16(const X_Polygon3* poly, X_Polygon3_fp16x16* dest)
{
    dest->totalVertices = poly->totalVertices;
    
    for(int i = 0; i < poly->totalVertices; ++i)
        poly->vertices[i] = x_vec3_to_vec3_fp16x16(poly->vertices + i);
}

void x_polygon3_fp16x16_to_polygon3(const X_Polygon3_fp16x16* poly, X_Polygon3* dest)
{
    dest->totalVertices = poly->totalVertices;
    
    for(int i = 0; i < poly->totalVertices; ++i)
        poly->vertices[i] = x_vec3_fp16x16_to_vec3(poly->vertices + i);
}

void x_polygon3_render_flat_shaded(X_Polygon3* poly, X_RenderContext* renderContext, X_Color color)
{
    X_Vec3 clippedV[X_POLYGON3_MAX_VERTS];
    X_Polygon3 clipped = x_polygon3_make(clippedV, X_POLYGON3_MAX_VERTS);
    
    if(!x_polygon3_clip_to_frustum(poly, renderContext->viewFrustum, &clipped))
        return;
    
    if(clipped.totalVertices != 3)
        return;
    
    X_TriangleFiller filler;
    x_trianglefiller_init(&filler, renderContext);
    
    for(int i = 0; i < 3; ++i)
    {
        X_Vec3 transformed;
        x_mat4x4_transform_vec3(renderContext->viewMatrix, clipped.vertices + i, &transformed);
        
        X_Vec2 projected;
        x_viewport_project_vec3(&renderContext->cam->viewport, &transformed, &projected);
        
        x_trianglefiller_set_flat_shaded_vertex(&filler, i, projected, transformed.z);
    }
    
    X_Plane plane;
    x_plane_init_from_three_points(&plane, poly->vertices + 0, poly->vertices + 1, poly->vertices + 2);
    
    if(!x_plane_point_is_on_normal_facing_side(&plane, &renderContext->camPos))
        return;
    
    x_fp16x16 dir = 46340;
    X_Vec3_fp16x16 light = {  dir, -dir, 0 }; 
    
    int shade = X_MAX(0, x_vec3_fp16x16_dot(&plane.normal, &light)) / (65536 / 16);
    
    x_trianglefiller_fill_flat_shaded(&filler, color);
}


