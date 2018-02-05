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

#include "X_BoundBox.h"
#include "X_Plane.h"
#include "X_Frustum.h"

X_BoundBoxPlaneFlags x_boundbox_determine_plane_clip_flags(X_BoundBox* box, X_Plane* plane)
{
    int px = (plane->normal.x > 0 ? 1 : 0);
    int py = (plane->normal.y > 0 ? 1 : 0);
    int pz = (plane->normal.z > 0 ? 1 : 0);
    
    X_Vec3 furthestPointAlongNormal = x_vec3_make(box->v[px].x, box->v[py].y, box->v[pz].z);
    if(!x_plane_point_is_on_normal_facing_side(plane, &furthestPointAlongNormal))
        return X_BOUNDBOX_OUTSIDE_PLANE;
    
    X_Vec3 closestPointAlongNormal = x_vec3_make(box->v[px ^ 1].x, box->v[py ^ 1].y, box->v[pz ^ 1].z);
    if(x_plane_point_is_on_normal_facing_side(plane, &closestPointAlongNormal))
        return X_BOUNDBOX_INSIDE_PLANE;
    
    return X_BOUNDBOX_INTERSECT_PLANE;
}

// Based on an algorithm described at http://www.txutxi.com/?p=584
X_BoundBoxFrustumFlags x_boundbox_determine_frustum_clip_flags(X_BoundBox* box, X_Frustum* frustum, X_BoundBoxFrustumFlags parentFlags)
{
    if(parentFlags == X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM)
        return X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM;
    
    X_BoundBoxFrustumFlags newFlags = (X_BoundBoxFrustumFlags)0;
    
    for(int i = 0; i < frustum->totalPlanes; ++i)
    {
        if(!x_boundbox_clip_against_frustum_plane(parentFlags, i))
            continue;
        
        X_BoundBoxPlaneFlags planeFlags = x_boundbox_determine_plane_clip_flags(box, frustum->planes + i);
        
        if(planeFlags == X_BOUNDBOX_OUTSIDE_PLANE)
            return X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM;
        
        if(planeFlags == X_BOUNDBOX_INTERSECT_PLANE)
            newFlags = (X_BoundBoxFrustumFlags)(newFlags | (1 << i));
    }
    
    return newFlags;
}

void x_boundbox_print(X_BoundBox* box)
{
    x_vec3_fp16x16_print(box->v + 0, "Mins");
    x_vec3_fp16x16_print(box->v + 1, "Maxs");
}

