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
#include "level/X_RayTracer.h"


static bool flag_enabled(X_BoxCollider* collider, X_BoxColliderFlags flag)
{
    return collider->flags & flag;
}

static void apply_velocity(X_BoxCollider* collider, Vec3* velocity)
{
    collider->velocity += *velocity;
}

static void apply_gravity(X_BoxCollider* collider)
{
    if(flag_enabled(collider, X_BOXCOLLIDER_APPLY_GRAVITY))
        apply_velocity(collider, collider->gravity);
}

static void apply_friction(X_BoxCollider* collider)
{
    x_fp16x16 currentSpeed = x_vec3_length(&collider->velocity);
    if(currentSpeed == 0)
        return;
    
    x_fp16x16 newSpeed = currentSpeed - collider->frictionCoefficient;
    newSpeed = x_fp16x16_clamp(newSpeed, 0, collider->maxSpeed);
    
    newSpeed = x_fp16x16_div(newSpeed, currentSpeed);
    
    // FIXME: write an actual scale vector function, this is hacky
    Vec3 origin = x_vec3_origin();
    collider->velocity = x_vec3_add_scaled(&origin, &collider->velocity, newSpeed);
}

static bool try_push_into_ground(X_BoxCollider* collider, X_BspLevel* level, x_fp16x16 distance, X_RayTracer* trace)
{
    Vec3 end = Vec3(collider->position.x, collider->position.y + distance, collider->position.z);
    x_raytracer_init(trace, level, x_bsplevel_get_level_model(level), &collider->position, &end, &collider->boundBox);
    return x_raytracer_trace(trace);
}

static bool plane_is_floor_surface(const Plane* plane)
{
    const x_fp16x16 MAX_FLOOR_Y_NORMAL = x_fp16x16_from_float(-0.7);
    return plane->normal.y <= MAX_FLOOR_Y_NORMAL;
}

static bool plane_is_vertical(const Plane* plane)
{
    return plane->normal.y == 0;
}

static bool on_floor(X_BoxCollider* collider, X_BspLevel* level, X_RayTracer* trace)
{
    const x_fp16x16 MAX_FLOOR_DISTANCE_EPSILON = x_fp16x16_from_float(3);
    return try_push_into_ground(collider, level, MAX_FLOOR_DISTANCE_EPSILON, trace)
        && plane_is_floor_surface(&trace->collisionPlane);
}

typedef enum IterationFlags
{
    IT_MOVE_SUCCESS = 1,
    IT_HIT_VERTICAL_WALL = 2,
    IT_ON_FLOOR = 4,
    IT_HIT_ANYTHING = 8,
    IT_HIT_PORTAL = 16
} IterationFlags;

static void adjust_velocity_to_slide_along_wall(Vec3* velocity, Plane* plane, x_fp16x16 bounceCoefficient)
{
    Vec3 temp = MakeVec3(plane->normal);

    x_fp16x16 dot = x_vec3_dot(&temp, velocity);
    *velocity = x_vec3_add_scaled(velocity, &temp, -x_fp16x16_mul(dot, bounceCoefficient));
}

static IterationFlags move_and_adjust_velocity(X_BoxCollider* collider, X_BspLevel* level, X_RayTracer* trace, Vec3* newVelocity, Vec3* newPos)
{
    Vec3fp vel = MakeVec3fp(collider->velocity);
    vel.normalize();

    Vec3fp newPosition = MakeVec3fp(*newPos) + vel * 30;

    Ray3 moveRay(MakeVec3fp(collider->position), newPosition);
    // Check for collision with portal
    for(Portal* portal = level->portalHead; portal != nullptr; portal = portal->next)
    {
        Ray3 clipRay;

        if(moveRay.clipToPlane(portal->plane, clipRay) == 1)
        {
            // We're going to hit the portal
            if(portal->pointInPortal(clipRay.v[1]))
            {
                // Did we actually make it through?
                Vec3fp newPosTemp = MakeVec3fp(*newPos);

                if(portal->plane.distanceTo(newPosTemp) > 0)
                {
                    // No didn't actually make it through, but probably closer than collision would
                    // normally let us get to the wall. But we need to be close to go through the
                    // portal, so let it happen.

                    return IT_MOVE_SUCCESS;
                }

                Vec3fp posTemp = MakeVec3fp(*newPos);

                *newPos = MakeVec3(portal->transformPointToOtherSide(posTemp));

                Vec3fp velocityTemp = MakeVec3fp(collider->velocity);
                
                Mat4x4 mat = portal->transformToOtherSide;
                mat.dropTranslation();
                mat.transpose3x3();

                Vec3fp newV = mat.transform(velocityTemp);

                *newVelocity = MakeVec3(newV);

                collider->collisionInfo.type = BOXCOLLIDER_COLLISION_PORTAL;
                collider->collisionInfo.hitPortal = portal;

                return IT_MOVE_SUCCESS;
            }
        }
    }

    x_raytracer_init(trace, level, x_bsplevel_get_level_model(level), &collider->position, newPos, &collider->boundBox);
    
    if(!x_raytracer_trace(trace))
        return IT_MOVE_SUCCESS;
    
    IterationFlags flags = (IterationFlags)0;
    if(plane_is_vertical(&trace->collisionPlane))
        flags = (IterationFlags)(flags | IT_HIT_VERTICAL_WALL);
    
    if(plane_is_floor_surface(&trace->collisionPlane))
        flags = (IterationFlags)(flags | IT_ON_FLOOR);
    
    adjust_velocity_to_slide_along_wall(newVelocity, &trace->collisionPlane, collider->bounceCoefficient);
    
    *newPos = trace->collisionPoint + *newVelocity;
    
    return flags;
}

static int run_move_iterations(X_BoxCollider* collider, X_BspLevel* level, X_RayTracer* lastHitWall)
{
    Vec3 newPos = collider->position + collider->velocity;
    Vec3 newVelocity = collider->velocity;
    int flags = 0;
    
    const int MAX_ITERATIONS = 4;
    int i;
    for(i = 0; i < MAX_ITERATIONS && !(flags & IT_MOVE_SUCCESS); ++i)
        flags |= move_and_adjust_velocity(collider, level, lastHitWall, &newVelocity, &newPos);
    
    collider->position = newPos;
    collider->velocity = newVelocity;
    
    return flags | (i != 0 ? IT_HIT_ANYTHING : 0);
}

static bool try_and_move_up_step(X_BoxCollider* collider, X_BspLevel* level, Vec3* oldPos, Vec3* oldVelocity, X_RayTracer* lastHitWallFromFirstAttempt)
{
    Vec3 adjustedPos = collider->position;
    Vec3 adjustedVelocity = collider->velocity;
    
    const x_fp16x16 MAX_STEP_SIZE = x_fp16x16_from_float(18.0);
    
    // Try and move the collider up the height of the step and reattempt the move
    collider->position = *oldPos;
    collider->position.y -= MAX_STEP_SIZE;
    
    collider->velocity = *oldVelocity;
    
    X_RayTracer lastHitWall;
    int flags = run_move_iterations(collider, level, &lastHitWall);
    bool successfullyClimbedStep = (flags & IT_MOVE_SUCCESS)
        && try_push_into_ground(collider, level, MAX_STEP_SIZE, &lastHitWall)
        && plane_is_floor_surface(&lastHitWall.collisionPlane);
    
    if(!successfullyClimbedStep)
    {
        collider->position = adjustedPos;
        collider->velocity = adjustedVelocity;
        return 0;
    }
    
    // We successfully moved up the step, but we don't know how big the step was, so use the hit point
    // with the floor to reposition the collider
    collider->position = lastHitWall.collisionPoint;
    *lastHitWallFromFirstAttempt = lastHitWall;
    return 1;
}

static void link_to_model_standing_on(X_BoxCollider* collider, X_BspModel* model)
{
    x_link_insert_after(&collider->objectsOnModel, &model->objectsOnModelHead);
}

int stick_to_floor(X_BoxCollider* collider, X_BspLevel* level)
{
    X_RayTracer floor;
    if(!on_floor(collider, level, &floor))
        return 0;
    
    collider->position = floor.collisionPoint;
    adjust_velocity_to_slide_along_wall(&collider->velocity, &floor.collisionPlane, collider->bounceCoefficient);
    link_to_model_standing_on(collider, floor.hitModel);
    
    return IT_ON_FLOOR;
}

static bool try_move(X_BoxCollider* collider, X_BspLevel* level)
{
    Vec3 oldVelocity = collider->velocity;
    Vec3 oldPos = collider->position;
    
    X_RayTracer lastHitWall;
    int flags = run_move_iterations(collider, level, &lastHitWall);
    
    if(flags & IT_HIT_VERTICAL_WALL)
    {
        if(try_and_move_up_step(collider, level, &oldPos, &oldVelocity, &lastHitWall))
            flags |= IT_MOVE_SUCCESS;
    }
    
    if(!(flags & IT_MOVE_SUCCESS))
    {
        collider->velocity = x_vec3_origin();
        collider->position = oldPos;
        return 0;
    }
    
    flags |= stick_to_floor(collider, level);
    
    if(flags & IT_ON_FLOOR)
    {
        apply_friction(collider);
        collider->flags |= X_BOXCOLLIDER_ON_GROUND;
    }
    else
    {
        collider->flags &= ~X_BOXCOLLIDER_ON_GROUND;
    }
    
    return 1;
}

void x_boxcollider_init(X_BoxCollider* collider, BoundBox* boundBox, X_BoxColliderFlags flags)
{
    static Vec3 gravity = { 0, x_fp16x16_from_float(0.25), 0 };
    
    collider->boundBox = *boundBox;
    collider->flags = flags;
    collider->gravity = &gravity;
    collider->frictionCoefficient = x_fp16x16_from_float(4.0);
    collider->maxSpeed = x_fp16x16_from_float(20.0);
    collider->bounceCoefficient = X_FP16x16_ONE;
    collider->collisionInfo.type = BOXCOLLIDER_COLLISION_NONE;
    
    x_link_init_self(&collider->objectsOnModel);
}

static void unlink_from_model_standing_on(X_BoxCollider* collider)
{
    x_link_unlink(&collider->objectsOnModel);
}

void x_boxcollider_update(X_BoxCollider* collider, X_BspLevel* level)
{
    collider->collisionInfo.type = BOXCOLLIDER_COLLISION_NONE;
    
    unlink_from_model_standing_on(collider);
    apply_gravity(collider);
    try_move(collider, level);
}

