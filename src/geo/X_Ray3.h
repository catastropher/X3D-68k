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

#pragma once

#include "X_Vec3.h"
#include "math/X_fix.h"
#include "render/X_Texture.h"

struct Plane;
struct X_Frustum;
struct X_RenderContext;

struct Ray3
{
    Ray3() { }
    Ray3(Vec3fp start, Vec3fp end)
    {
        v[0] = start;
        v[1] = end;
    }

    bool clipToPlane(const Plane& plane, Ray3& dest);
    bool clipToFrustum(const X_Frustum& frustum, Ray3& dest) const;
    void render(const X_RenderContext& renderContext, X_Color color) const;

    Vec3fp lerp(fp t) const
    {
        return v[0] + (v[1] - v[0]) * t;
    }

    Vec3fp v[2];
};

