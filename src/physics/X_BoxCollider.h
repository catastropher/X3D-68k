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

#include "geo/X_Vec3.h"
#include "level/X_BspLevel.h"
#include "memory/X_Link.h"
#include "memory/X_BitSet.hpp"
#include "entity/Component.hpp"

struct X_RayTracer;

typedef enum X_BoxColliderFlags
{
    X_BOXCOLLIDER_APPLY_GRAVITY = 1,
    X_BOXCOLLIDER_APPLY_FRICTION = 2,
    X_BOXCOLLIDER_ON_GROUND = 4,
    BOXCOLLIDER_IN_PORTAL = 8
} X_BoxColliderFlags;

enum BoxColliderCollisionType
{
    BOXCOLLIDER_COLLISION_NONE,
    BOXCOLLIDER_COLLISION_PORTAL
};

struct BoxColliderCollisionInfo
{
    BoxColliderCollisionType type;

    union
    {
        Portal* hitPortal;
    };
};

struct X_BoxCollider;

void x_boxcollider_init(X_BoxCollider* collider, BoundBox* boundBox, EnumBitSet<X_BoxColliderFlags> flags);

struct X_BoxCollider
{
    X_BoxCollider()
    {
        static BoundBox box;
        x_boxcollider_init(this, &box, X_BOXCOLLIDER_APPLY_GRAVITY);
    }
    
    bool traceRay(X_RayTracer& tracer);
    
    void applyFrameVelocity(const Vec3fp& velocity_)
    {
        frameVelocity = velocity_;
    }
    
    void applyImpulseVelocity(const Vec3fp& velocity_)
    {
        impulseVelocity = velocity_;
    }

    EnumBitSet<X_BoxColliderFlags> flags;

    BoundBox boundBox;
    int levelCollisionHull;
    Vec3fp velocity;
    Vec3fp frameVelocity;       // Velocity for the entire duration of the frame
    Vec3fp impulseVelocity;     // Single time applied velocity
    Vec3fp* gravity;
    fp bounceCoefficient;
    fp frictionCoefficient;
    fp maxSpeed;
    BoxColliderCollisionInfo collisionInfo;
    int transformComponentId = COMPONENT_INVALID_ID;
    
    Portal* currentPortal;
    
    X_Link objectsOnModel;
};

static inline bool x_boxcollider_is_on_ground(X_BoxCollider* collider)
{
    return collider->flags.hasFlag(X_BOXCOLLIDER_ON_GROUND);
}

//static inline void x_boxcollider_apply_velocity(X_BoxCollider* collider, X_Vec3_fp16x16*)

