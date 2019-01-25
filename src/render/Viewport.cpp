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

#include "Viewport.hpp"
#include "math/Trig.hpp"
#include "util/Util.hpp"

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

void Viewport::init(Vec2 screenPos_, int w_, int h_, fp fieldOfView)
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

    // Order has to be left, right, bottom, top
    int id[4] = { 1, 2, 0, 3 };

    // Top, bottom, left, and right planes
    for(int i = 0; i < 4; ++i)
    {
        int next = (i != 3 ? i + 1 : 0);
        viewFrustumPlanes[id[i]] = FrustumPlane(nearPlaneVertices[i], camPos, nearPlaneVertices[next], id[i]);
    }

    // Near plane
    fp fakeDistToNearPlane = fp::fromFloat(0.5);       // TODO: what should this value really be?
    Vec3fp pointOnNearPlane = camPos + forward * fakeDistToNearPlane;

    viewFrustumPlanes[4] = FrustumPlane(forward, pointOnNearPlane, 4);

    // Far plane
    Vec3fp backward = -forward;
    Vec3fp pointOnFarPlane = camPos + forward * fp::fromInt(1000);

    viewFrustumPlanes[5] = FrustumPlane(backward, pointOnFarPlane, 5);
}

fp recip(fp x)
{
    //return fp::fromFloat(1.0 / x.toFloat());

    if(x > fp::fromFloat(1.5))
    {
        return fp::fromInt(1) / x;
    }

    x = x - fp::fromInt(1);

    fp sum = fp::fromInt(1) - x;

    fp newX = x * x;
    sum += newX;

    newX = newX * x;
    sum = sum - newX;

    newX = newX * x;
    sum = sum + newX;

    return sum;
}

void Viewport::project(const Vec3fp& src, Vec2_fp16x16& dest)
{
#if 0
    Vec3fp low(0, 0, 0);
    Vec3fp hi = src;

    Vec3fp mid;

    fp near = fp::fromFloat(1.0);
    fp epsilon = fp::fromFloat(0.001);

    int it = 0;

    int shift = 16 - __builtin_clz(src.z.toFp16x16());

    while(hi.z > fp::fromInt(2))
    {
        hi = hi / 2;
    }

    if(hi.z <= fp::fromInt(1))
    {
        printf("Invalid val: %d\n", hi.z.toFloat());
    }


    //if(shift >= 7) shift = 7;
    // if(shift < 0) shift = 0;

    // hi = hi / (1 << shift);

    fp val = hi.z;
    fp x;


  

    //fp x = x + mul(x, ONE - mul(val, x));

    //printf("Shift: %d, Z = %f\n", shift, hi.z.toFloat());

    // for(int i = 0; i < 20; ++i)
    // {
    //     mid = (low + hi) / 2;
    //      ++it;

    //     if(abs(mid.z - near) < epsilon)
    //     {
    //         break;
    //     }

    //     if(mid.z > near)
    //     {
    //         hi = mid;
    //     }
    //     else
    //     {
    //         low = mid;
    //     }
    // }

    //printf("Val: %f, original: %f\n", val.toFloat(), src.z.toFloat());

    int shiftDown = 0;
    //int invZ = (fp::fromInt(1) / hi.z).toFp16x16();  //x_fastrecip((val / 8).toFp16x16());  //x_fastrecip_unshift(val.toFp16x16(), shiftDown);

    int invZ = recip(hi.z).toFp16x16();

    mid.x = ((long long)hi.x.toFp16x16() * invZ) >> (shiftDown + 16);
    mid.y = ((long long)hi.y.toFp16x16() * invZ) >> (shiftDown + 16);

      if(src.z < fp::fromFloat(100))
    {
        goto small;
    }


    // do
    // {
    //     mid = (low + hi) / 2;
    //     ++it;

    //     if(abs(mid.z - near) < epsilon)
    //     {
    //         break;
    //     }

    //     if(mid.z > near)
    //     {
    //         hi = mid;
    //     }
    //     else
    //     {
    //         low = mid;
    //     }

    // } while(true);

    //printf("It: %d\n", it);

    dest.x = (mid.x * distToNearPlane + fp::fromInt(w / 2)).toFp16x16();
    dest.y = (mid.y * distToNearPlane + fp::fromInt(h / 2)).toFp16x16();

    return;


#endif

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

void Viewport::clamp(Vec2& v)
{
    v.x = std::max(v.x, screenPos.x);
    v.x = std::min(v.x, screenPos.x + w - 1);

    v.y = std::max(v.y, screenPos.y);
    v.y = std::min(v.y, screenPos.y + h - 1);
}

void Viewport::clampfp(Vec2_fp16x16& v)
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

#include "geo/Ray3.hpp"

void Viewport::projectBisect(const Vec3fp& src, Vec2_fp16x16& dest)
{
#if 0
    fp dist = fp::fromFloat(1.0);
    Plane plane;

    plane.normal = Vec3fp(0, 0, -fp::fromInt(1));
    plane.d = dist;

    Ray3 ray(Vec3fp(0, 0, 0), src);


    ray.clipToPlane(plane, ray);

    dest.x = (ray.v[1].x * distToNearPlane + fp::fromInt(w / 2)).toFp16x16();
    dest.y = (ray.v[1].y * distToNearPlane + fp::fromInt(h / 2)).toFp16x16();
#endif
    

    Vec3fp low(0, 0, 0);
    Vec3fp hi = src;

    Vec3fp mid;

    fp near = fp::fromFloat(1.0);
    fp epsilon = fp::fromFloat(0.001);

    do
    {
        mid = (low + hi) / 2;

        if(abs(mid.z - near) < epsilon)
        {
            break;
        }

        if(mid.z > near)
        {
            hi = mid;
        }
        else
        {
            low = mid;
        }

    } while(true);

    dest.x = (mid.x * distToNearPlane + fp::fromInt(w / 2)).toFp16x16();
    dest.y = (mid.y * distToNearPlane + fp::fromInt(h / 2)).toFp16x16();
}

