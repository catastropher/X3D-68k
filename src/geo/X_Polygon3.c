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

