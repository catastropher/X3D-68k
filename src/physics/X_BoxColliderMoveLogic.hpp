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
#include "geo/X_Ray3.h"
#include "geo/X_Vec3.h"
#include "level/X_BspLevel.h"
#include "level/X_BspRayTracer.hpp"
#include "memory/X_BitSet.hpp"

typedef enum IterationFlags
{
    IT_MOVE_SUCCESS = 1,
    IT_HIT_VERTICAL_WALL = 2,
    IT_ON_FLOOR = 4,
    IT_HIT_ANYTHING = 8,
    IT_HIT_PORTAL = 16,
    IT_ENTER_PORTAL_BOX = 32
} IterationFlags;


class BoxColliderMoveLogic
{
public:
    BoxColliderMoveLogic(X_BoxCollider& collider_, X_BspLevel& level_, Vec3fp& startingPosition_, Vec3fp& velocity_)
        : collider(collider_),
        level(level_),
        startingPosition(startingPosition_),
        velocity(velocity_),
        flags(0)
    {

    }

    bool tryMoveNormally();
    bool tryMoveUpStep();

    EnumBitSet<IterationFlags> getMovementFlags()
    {
        return flags;
    }

    Vec3fp getFinalPosition()
    {
        return newPosition;
    }

    Vec3fp getFinalVelocity()
    {
        return velocity;
    }

    RayCollision<int> getLastHitWall()
    {
        return lastHitWall;
    }

    bool potentiallyHitStep()
    {
        return flags.hasFlag(IT_HIT_VERTICAL_WALL);
    }

private:
    void moveAndAdjustVelocity();

    void slideAlongWall();
    void comeToDeadStop();
    void tryStickToFloor();

    Ray3 getMovementRay();
    void runMoveIterations();

    bool tryPushIntoGround(fp distance);
    bool onFloor();

    bool traceRay(const Ray3& ray, RayCollision<int>& collision);

    Portal* findWhichPortalBoxInside();

    X_BoxCollider& collider;
    X_BspLevel& level;

    RayCollision<int> lastHitWall;

    Vec3fp startingPosition;
    Vec3fp newPosition;
    Vec3fp velocity;
    EnumBitSet<IterationFlags> flags;
};