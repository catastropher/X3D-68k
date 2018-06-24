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
#include "X_Frustum.h"
#include "math/X_Mat4x4.h"
#include "object/X_CameraObject.h"

bool Ray3::clipToPlane(const Plane& plane, Ray3& dest)
{
    fp v0DistToPlane = plane.distanceTo(v[0]);
    bool v0In = v0DistToPlane > 0;
    
    fp v1DistToPlane = plane.distanceTo(v[1]);
    bool v1In = v1DistToPlane > 0;
    
    // Trivial case: both points inside
    if(v0In && v1In)
    {
        dest = *this;

        return true;
    }
    
    // Trivial case: both points outside
    if(!v0In && !v1In)
    {
        return false;
    }
    
    if(!v0In)
    {
        std::swap(v[0], v[1]);
        std::swap(v0DistToPlane, v1DistToPlane);
    }

    // One inside and one outside, so need to clip
    fp t = v0DistToPlane / (v0DistToPlane - v1DistToPlane);
    
    dest.v[1] = lerp(t);
    dest.v[0] = v[0];
    
    return true;
}

bool Ray3::clipToFrustum(const X_Frustum& frustum, Ray3& dest) const
{
    dest = *this;
    
    for(int i = 0; i < frustum.totalPlanes ;++i)
    {
        if(!dest.clipToPlane(frustum.planes[i], dest))
        {
            return false;
        }
    }
    
    return true;
}

void Ray3::render(const X_RenderContext& renderContext, X_Color color) const
{
    Ray3 clipped = *this;
    if(!clipToFrustum(*renderContext.viewFrustum, clipped))
    {
        return;
    }

    Ray3 transformed;
    for(int i = 0; i < 2; ++i)
    {
        transformed.v[i] = renderContext.viewMatrix->transform(clipped.v[i]);
    }
    
    if(transformed.v[0].z <= 0 || transformed.v[0].z <= 0)
    {
        return;
    }
    
    X_Vec2 projected[2];
    for(int i = 0; i < 2; ++i)
    {
        renderContext.cam->viewport.project(transformed.v[i], projected[i]);

        // FIXME
        projected[i].x = projected[i].x >> 16;
        projected[i].y = projected[i].y >> 16;
    }

    renderContext.canvas->drawLine(projected[0], projected[1], color);
}

