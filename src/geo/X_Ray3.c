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

#include "X_Ray3.h"
#include "X_Plane.h"
#include "render/X_RenderContext.h"

_Bool x_ray3_clip_to_plane(const X_Ray3* ray, const X_Plane* plane, X_Ray3* dest)
{
    int v0DistToPlane = x_plane_point_distance(plane, ray->v + 0) >> 16;
    _Bool v0In = v0DistToPlane > 0;
    
    int v1DistToPlane = x_plane_point_distance(plane, ray->v + 1) >> 16;
    _Bool v1In = v1DistToPlane > 0;
    
    // Trivial case: both points inside
    if(v0In && v1In)
    {
        *dest = *ray;
        return 1;
    }
    
    // Trivial case: both points outside
    if(!v0In && !v1In)
        return 0;
    
    // One inside and one outside, so need to clip
    if(v0In)
    {
        x_fp16x16 t = (v0DistToPlane << 16) / (v0DistToPlane - v1DistToPlane);
        
        dest->v[0] = ray->v[0];
        x_ray3_lerp(ray, t, dest->v + 1);
        
        return 1;
    }
    
    x_fp16x16 t = (v1DistToPlane << 16) / (v1DistToPlane - v0DistToPlane);
    
    dest->v[1] = ray->v[1];
    x_ray3_lerp(ray, X_FP16x16_ONE - t, dest->v + 0);
    
    return 1;
}

_Bool x_ray3_clip_to_frustum(const X_Ray3* ray, const X_Frustum* frustum, X_Ray3* dest)
{
    _Bool inside = 1;
    *dest = *ray;
    
    for(int i = 0; i < frustum->totalPlanes && inside; ++i)
    {
        inside &= x_ray3_clip_to_plane(dest, frustum->planes + i, dest);
        //printf("Fail by plane %d\n", i);
    }
    
    return inside;
}

void x_ray3d_render(const X_Ray3* ray, X_RenderContext* rcontext, X_Color color)
{
    X_Ray3 clipped;
    if(!x_ray3_clip_to_frustum(ray, rcontext->viewFrustum, &clipped))
        return;
    
    X_Ray3 transformed;
    for(int i = 0; i < 2; ++i)
        x_mat4x4_transform_vec3(rcontext->viewMatrix, clipped.v + i, transformed.v + i);
    
    X_Vec2 projected[2];
    for(int i = 0; i < 2; ++i)
        x_viewport_project_vec3(&rcontext->cam->viewport, transformed.v + i, projected + i);
    
    x_viewport_clamp_vec2(&rcontext->cam->viewport, projected + 0);
    x_viewport_clamp_vec2(&rcontext->cam->viewport, projected + 1);
    
    x_texture_draw_line(&rcontext->canvas->tex, projected[0], projected[1], color);
}

