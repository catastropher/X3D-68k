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

#include "Ray3.hpp"
#include "Plane.hpp"
#include "render/RenderContext.hpp"
#include "render/Renderer.hpp"
#include "Frustum.hpp"
#include "math/Mat4x4.hpp"
#include "object/CameraObject.hpp"

int Ray3::clipToPlane(const Plane& plane, Ray3& dest)
{
    fp v0DistToPlane = plane.distanceTo(v[0]);
    int v0In = v0DistToPlane >= 0;
    
    fp v1DistToPlane = plane.distanceTo(v[1]);
    int v1In = v1DistToPlane >= 0;

    int flags = v0In | (v1In << 1);
    fp t;

    switch(flags)
    {
        case 0:
            break;

        case 1:
            t = v0DistToPlane / (v0DistToPlane - v1DistToPlane);
    
            dest.v[1] = lerp(t);
            dest.v[0] = v[0];

            break;

        case 2:
            t = v1DistToPlane / (v1DistToPlane - v0DistToPlane);
    
            dest.v[0] = lerp(fp(X_FP16x16_ONE) - t);
            dest.v[1] = v[1];

            break;

        case 3:
            dest = *this;

            break;
    }

    return flags;
}

bool Ray3::clipToFrustum(const X_Frustum& frustum, Ray3& dest) const
{
    dest = *this;

    for(int i = 0; i < frustum.totalPlanes ;++i)
    {
        if(dest.clipToPlane(frustum.planes[i], dest) == 0)
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
    
    // if(transformed.v[0].z <= 0 || transformed.v[0].z <= 0)
    // {
    //     return;
    // }
    
    Vec2 projected[2];
    for(int i = 0; i < 2; ++i)
    {
        renderContext.cam->viewport.project(transformed.v[i], projected[i]);

        // FIXME
        projected[i].x = projected[i].x >> 16;
        projected[i].y = projected[i].y >> 16;
    }

    renderContext.canvas->drawLine(projected[0], projected[1], color);
}

void Ray3::renderShaded(const X_RenderContext& renderContext, X_Color color, fp maxDist) const
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

    if(transformed.v[0].z > maxDist && transformed.v[1].z > maxDist)
    {
        return;
    }
    
    Vec2 projected[2];
    fp intensity[2];

    for(int i = 0; i < 2; ++i)
    {
        intensity[i] = fp::fromInt(1) - transformed.v[i].z / maxDist.toInt();

        renderContext.cam->viewport.projectBisect(transformed.v[i], projected[i]);

        // FIXME
        projected[i].x = projected[i].x >> 16;
        projected[i].y = projected[i].y >> 16;
    }

    renderContext.canvas->drawLineShaded(projected[0], projected[1], color, intensity[0], intensity[1], renderContext.renderer->colorMap);
}

