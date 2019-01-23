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

#include "Texture.hpp"
#include "geo/Vec3.hpp"
#include "math/FixedPoint.hpp"

typedef enum X_LightFlags
{
    X_LIGHT_FREE = 1,
    X_LIGHT_ENABLED = 2
} X_LightFlags;

typedef struct X_Light
{
    int id;
    X_LightFlags flags;
    Vec3 position;
    Vec3 direction;
    x_fp24x8 intensity;
} X_Light;

static inline bool x_light_is_free(X_Light* light)
{
    return light->flags & X_LIGHT_FREE;
}

static inline bool x_light_is_enabled(X_Light* light)
{
    return light->flags & X_LIGHT_ENABLED;
}

