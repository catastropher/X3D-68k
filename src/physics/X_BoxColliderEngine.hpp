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

#include "X_BoxCollider.h"
#include "level/X_BspRayTracer.hpp"
#include "geo/X_Vec3.h"

using BoxRayTracer = BspRayTracer<int, X_BspClipNode*>;

typedef enum IterationFlags
{
    IT_MOVE_SUCCESS = 1,
    IT_HIT_VERTICAL_WALL = 2,
    IT_ON_FLOOR = 4,
    IT_HIT_ANYTHING = 8,
    IT_HIT_PORTAL = 16
} IterationFlags;

struct BoxColliderState
{
    BoxColliderState(Vec3fp& position_, Vec3fp& velocity_)
        : position(position_),
        velocity(velocity_),
        flags(0)
    {
        newPosition = position + velocity;
    }

    Vec3fp position;
    Vec3fp newPosition;
    Vec3fp velocity;
    EnumBitSet<IterationFlags> flags;
};

class BoxColliderEngine
{
public:
    BoxColliderEngine(X_BoxCollider& collider_, X_BspLevel& level_)
        : collider(collider_),
        level(level_)
    {

    }

    void step();

private:
    void resetCollisionState();

    void unlinkFromModelStandingOn();
    void linkToModelStandingOn();

    void applyGravity();
    void applyFriction(BoxColliderState& state);

    bool tryMove();
    void runMoveIterations(BoxColliderState& state);
    bool tryMoveUpStep(BoxColliderState& state);
    void moveAndAdjustVelocity(BoxColliderState& state);
    void slideAlongWall(BoxColliderState& state);
    void comeToDeadStop(BoxColliderState& state);
    bool tryPushIntoGround(Vec3fp& startLocation, fp distance);
    void tryStickToFloor(BoxColliderState& state);

    bool onFloor(BoxColliderState& state);

    bool traceRay(const Ray3& ray, RayCollision<int>& collision);

    Ray3 getMovementRay(BoxColliderState& state);

    X_BoxCollider& collider;
    X_BspLevel& level;

    RayCollision<int> lastHitWall;
};

