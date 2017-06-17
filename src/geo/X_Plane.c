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

#include "X_Plane.h"

void x_plane_init_from_three_points(X_Plane* plane, const X_Vec3* a, const X_Vec3* b, const X_Vec3* c)
{
    X_Vec3 v1 = x_vec3_sub(a, b);
    X_Vec3 v2 = x_vec3_sub(c, b);
    
    plane->normal = x_vec3_cross(&v1, &v2);
    x_vec3_fp16x16_normalize(&plane->normal);
    
    x_plane_init_from_normal_and_point(plane, &plane->normal, a);
}

_Bool x_plane_clip_ray3d(const X_Plane* plane, const X_Ray3* src, X_Ray3* dest)
{
    int v0DistToPlane = x_plane_point_distance(plane, src->v + 0);
    _Bool v0In = v0DistToPlane > 0;
    
    int v1DistToPlane = x_plane_point_distance(plane, src->v + 1);
    _Bool v1In = v1DistToPlane > 0;
    
    // Trivial case: both points inside
    if(v0In && v1In)
    {
        *dest = *src;
        return 1;
    }
    
    // Trivial case: both points outside
    if(!v0In && !v1In)
        return 0;
    
    // One inside and one outside, so need to clip
    if(v1In)
    {
        x_fp16x16 t = (v0DistToPlane << 16) / (v0DistToPlane - v1DistToPlane);
        dest->v[0] = src->v[0];
        x_ray3d_lerp(src, t, dest->v + 1);
        return 1;
    }
    
    x_fp16x16 t = (v1DistToPlane << 16) / (v1DistToPlane - v0DistToPlane);
    dest->v[1] = src->v[1];
    x_ray3d_lerp(src, X_FP16x16_ONE - t, dest->v + 0);
    return 1;
}

