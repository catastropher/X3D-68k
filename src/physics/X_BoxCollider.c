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

#include "X_BoxCollider.h"

void x_boxcollider_init(X_BoxCollider* collider)
{
    
}

static _Bool flag_enabled(X_BoxCollider* collider, X_BoxColliderFlags flag)
{
    return collider->flags & flag;
}

static void apply_velocity(X_BoxCollider* collider, X_Vec3_fp16x16* velocity)
{
    collider->velocity = x_vec3_add(&collider->velocity, velocity);
}

static void apply_gravity(X_BoxCollider* collider)
{
    if(flag_enabled(collider, X_BOXCOLLIDER_APPLY_GRAVITY))
        apply_velocity(collider, collider->gravity);
}

void x_boxcollider_update(X_BoxCollider* collider, X_BspLevel* level)
{
    apply_gravity(collider);
}

