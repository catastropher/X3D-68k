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

#include "X_Viewport.h"
#include "math/X_trig.h"
#include "util/X_util.h"

void Viewport::initMipDistances()
{
    fp mipScale[3] =
    {
        fp::fromFloat(1.0),
        fp::fromFloat(0.5 * 0.8),
        fp::fromFloat(0.25 * 0.8)
    };

    // TODO: this will need to take into account y scale once it's added
    fp xScale = fp::fromInt(distToNearPlane);
    for(int i = 0; i < 3; ++i)
    {
        mipDistances[i] = xScale / mipScale[i];
    }
}

void Viewport::init(X_Vec2 screenPos_, int w_, int h_, fp fieldOfView)
{
    screenPos = screenPos_;
    w = w_;
    h = h_;
    distToNearPlane = (fp::fromInt(w / 2) / x_tan(fieldOfView / 2)).toInt();

    /// @todo If we add a far plane, this should be 6
    viewFrustum.totalPlanes = 4;
    viewFrustum.planes = viewFrustumPlanes;

    initMipDistances();
}

void Viewport::updateFrustum(const Vec3fp& camPos, const Vec3fp& forward, const Vec3fp& right, const Vec3fp& up)
{
    Vec3fp nearPlaneCenter = camPos + forward * distToNearPlane;

    int epsilon = 0;

    Vec3fp rightTranslation = right * (w / 2 + epsilon);
    Vec3fp upTranslation = up * (h / 2 + epsilon);

    Vec3fp nearPlaneVertices[4] =
    {
        nearPlaneCenter + rightTranslation + upTranslation,     // Right
        nearPlaneCenter + rightTranslation - upTranslation,     // Bottom
        nearPlaneCenter - rightTranslation - upTranslation,     // Left
        nearPlaneCenter - rightTranslation + upTranslation      // Top
    };

    // Top, bottom, left, and right planes
    for(int i = 0; i < 4; ++i)
    {
        int next = (i != 3 ? i + 1 : 0);
        viewFrustumPlanes[i] = Plane(nearPlaneVertices[i], camPos, nearPlaneVertices[next]);
    }

    // Near plane
    fp fakeDistToNearPlane = fp::fromFloat(0.5);       // TODO: what should this value really be?
    Vec3fp pointOnNearPlane = camPos + forward * fakeDistToNearPlane;

    viewFrustumPlanes[4] = Plane(forward, pointOnNearPlane);

    // Far plane
    Vec3fp backward = -forward;
    Vec3fp pointOnFarPlane = camPos + forward * fp::fromInt(1000);

    viewFrustumPlanes[5] = Plane(backward, pointOnFarPlane);
}

void Viewport::project(const Vec3fp& src, X_Vec2_fp16x16& dest)
{
    // TODO: may be able to get away with multiplying by distToNearPlane / z

    if(src.z < fp::fromFloat(100))
    {
        dest.x = (src.x / src.z * distToNearPlane + fp::fromInt(w / 2)).toFp16x16();
        dest.y = (src.y / src.z * distToNearPlane + fp::fromInt(h / 2)).toFp16x16();
    }
    else
    {
        int shiftDown; //16 + shift;// + 16 - shiftUp;
        int invZ = x_fastrecip_unshift(src.z.toFp16x16(), shiftDown); //x_fastrecip(z); //x_fastrecip_unshift(z, shiftUp);  //

        fp x = (((long long)src.x.toFp16x16() * invZ) >> (shiftDown + 16));
        fp y = (((long long)src.y.toFp16x16() * invZ) >> (shiftDown + 16));

        dest.x = (x * distToNearPlane + fp::fromInt(w / 2)).toFp16x16();
        dest.y = (y * distToNearPlane + fp::fromInt(h / 2)).toFp16x16();


        //fp inverseZ = fp::fromInt(distToNearPlane) / fp(src.z);   //fp(x_fastrecip(src.z.toFp16x16() >> 16));

        //dest.x = (src.x * inverseZ + fp::fromInt(w / 2)).toFp16x16();
        //dest.y = (src.y * inverseZ + fp::fromInt(h / 2)).toFp16x16();
    }
}

void Viewport::clamp(X_Vec2& v)
{
    v.x = std::max(v.x, screenPos.x);
    v.x = std::min(v.x, screenPos.x + w - 1);

    v.y = std::max(v.y, screenPos.y);
    v.y = std::min(v.y, screenPos.y + h - 1);
}

void Viewport::clampfp(X_Vec2_fp16x16& v)
{
    fp x = fp(v.x);
    fp y = fp(v.y);

    x = std::max(x, fp::fromInt(screenPos.x));
    x = std::min(x, fp::fromInt(screenPos.x + w - 1));

    y = std::max(y, fp::fromInt(screenPos.y));
    y = std::min(y, fp::fromInt(screenPos.y + h - 1));

    v.x = x.toFp16x16();
    v.y = y.toFp16x16();
}
