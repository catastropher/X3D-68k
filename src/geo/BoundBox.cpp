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

#include "BoundBox.hpp"
#include "Plane.hpp"
#include "Frustum.hpp"

template<>
BoundBoxPlaneFlags BoundBox::determinePlaneClipFlags(const Plane& plane) const
{
    int px = (plane.normal.x > 0 ? 1 : 0);
    int py = (plane.normal.y > 0 ? 1 : 0);
    int pz = (plane.normal.z > 0 ? 1 : 0);

    // FIXME: this only works because the bound box is really storing a Vec3fp as a Vec3i
    Vec3fp furthestPointAlongNormal = Vec3fp(v[px].x, v[py].y, v[pz].z);

    if(!plane.pointOnNormalFacingSide(furthestPointAlongNormal))
    {
        return X_BOUNDBOX_OUTSIDE_PLANE;
    }
    
    Vec3 closestPointAlongNormal = Vec3(v[px ^ 1].x, v[py ^ 1].y, v[pz ^ 1].z);
    Vec3fp closestPointAlongNormalTemp = MakeVec3fp(closestPointAlongNormal);

    if(plane.pointOnNormalFacingSide(closestPointAlongNormalTemp))
    {
        return X_BOUNDBOX_INSIDE_PLANE;
    }
    
    return X_BOUNDBOX_INTERSECT_PLANE;
}

// Based on an algorithm described at http://www.txutxi.com/?p=584
template<>
BoundBoxFrustumFlags BoundBox::determineFrustumClipFlags(const X_Frustum& frustum, BoundBoxFrustumFlags parentFlags) const
{
    if(parentFlags == X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM)
        return X_BOUNDBOX_TOTALLY_INSIDE_FRUSTUM;
    
    BoundBoxFrustumFlags newFlags = (BoundBoxFrustumFlags)0;
    
    for(int i = 0; i < frustum.totalPlanes; ++i)
    {
        if(!BoundBox::clipAgainstFrustumPlane(parentFlags, i))
        {
            continue;
        }
        
        BoundBoxPlaneFlags planeFlags = determinePlaneClipFlags(frustum.planes[i]);
        
        if(planeFlags == X_BOUNDBOX_OUTSIDE_PLANE)
        {
            return X_BOUNDBOX_TOTALLY_OUTSIDE_FRUSTUM;
        }
        
        if(planeFlags == X_BOUNDBOX_INTERSECT_PLANE)
        {
            newFlags = (BoundBoxFrustumFlags)(newFlags | (1 << i));
        }
    }
    
    return newFlags;
}

template<>
void BoundBox::print() const
{
    v[0].print("Min");
    v[1].print("Max");
}

