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
#include "level/X_BspRayTracer.hpp"
#include "level/X_Portal.hpp"

static bool frameByFrame = false;
static bool lastDown = false;

#include <SDL/SDL.h>

bool X_BoxCollider::traceRay(X_RayTracer& tracer)
{
    if(false && flags.isSet(BOXCOLLIDER_IN_PORTAL) && currentPortal != nullptr)
    {
        BspRayTracer<int, X_BspClipNode*> bspRayTracer(tracer.ray, tracer.level, 0);

        bool hitSomething = bspRayTracer.traceModel(currentPortal->bridgeModel);

        if(hitSomething && bspRayTracer.getCollision().hitNode == 5)
        {
            printf("Teleport to other side!\n");
        }

        x_raytracer_from_bspraytracer(bspRayTracer, tracer, tracer.level);

        return hitSomething;
    }
    else
    {
        return x_raytracer_trace(&tracer);
    }
}

static bool flag_enabled(X_BoxCollider* collider, X_BoxColliderFlags flag)
{
    return collider->flags.isSet(flag);
}

static void apply_velocity(X_BoxCollider* collider, Vec3fp* velocity)
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
    fp currentSpeed = collider->velocity.length();
    if(currentSpeed == 0)
        return;
    
    fp newSpeed = currentSpeed - collider->frictionCoefficient;
    newSpeed = clamp(newSpeed, fp::fromInt(0), collider->maxSpeed);
    
    newSpeed = newSpeed / currentSpeed;

    collider->velocity = collider->velocity * newSpeed;
}

static bool try_push_into_ground(X_BoxCollider* collider, X_BspLevel* level, fp distance, X_RayTracer* trace)
{
    Vec3fp end = Vec3fp(collider->position.x, collider->position.y + distance, collider->position.z);

    Vec3 startTemp = MakeVec3(collider->position);
    Vec3 endTemp = MakeVec3(end);

    x_raytracer_init(trace, level, x_bsplevel_get_level_model(level), &startTemp, &endTemp, &collider->boundBox);

    return collider->traceRay(*trace);
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

static void adjust_velocity_to_slide_along_wall(Vec3fp* velocity, Plane* plane, fp bounceCoefficient)
{
    *velocity = *velocity + plane->normal * (-plane->normal.dot(*velocity) * bounceCoefficient); 
}

void moveUpUntilInLevel(X_BspLevel* level, Vec3* pos)
{
    Vec3fp posTemp = MakeVec3fp(*pos);

    for(int i = 0; i < 100; ++i)
    {
        if(!level->findLeafPointIsIn(posTemp)->isOutsideLevel())
        {
            printf("Move up %d\n", i);
            *pos = MakeVec3(posTemp);
            return;
        }

        posTemp.y -= fp::fromFloat(1);
    }

    printf("Fail to move up\n");
}

static int move_and_adjust_velocity(X_BoxCollider* collider, X_BspLevel* level, X_RayTracer* trace, Vec3fp* newVelocity, Vec3fp* newPos)
{
    Vec3 newPosTemp = MakeVec3(*newPos);
    Vec3 posTemp = MakeVec3(collider->position);

    x_raytracer_init(trace, level, x_bsplevel_get_level_model(level), &posTemp, &newPosTemp, &collider->boundBox);
    
    if(!collider->traceRay(*trace))
        return IT_MOVE_SUCCESS;

    // Maybe we actually hit a portal...
    Vec3fp vel = collider->velocity;
    vel.normalize();

    Vec3fp newPosition = *newPos + vel * 40;

    Ray3 moveRay(collider->position, newPosition);
    // Check for collision with portal
    for(Portal* portal = level->portalHead; portal != nullptr; portal = portal->next)
    {
        collider->flags.set(BOXCOLLIDER_IN_PORTAL);
            collider->currentPortal = portal;
            break;

        Ray3 clipRay;

        if(moveRay.clipToPlane(portal->plane, clipRay) == 1)
        {

            printf("Going to hit portal\n");
            frameByFrame = true;
            // We're going to hit the portal
            if(portal->pointInPortal(clipRay.v[1]))
            {
                // Did we actually make it through?
                Vec3fp newPosTemp = *newPos;

                printf("Distance: %f\n", portal->plane.distanceTo(newPosTemp).toFloat());

                if(portal->plane.distanceTo(newPosTemp) > 0)
                {
                    // No didn't actually make it through, but probably closer than collision would
                    // normally let us get to the wall. But we need to be close to go through the
                    // portal, so let it happen.

                    //moveUpUntilInLevel(level, newPos);

                    printf("HERE\n");

                    collider->position.y = portal->center.y.toFp16x16();
                    newPos->y = portal->center.y.toFp16x16();

                    trace->collisionPlane.normal.print("Collision normal");

                    // Only allow if would have hit the wall
                    if(trace->collisionPlane.normal != portal->plane.normal)
                    {
                        printf("Wrong normal...\n");
                        continue;
                    }
                    else
                    {
                        //printf("Not wrong norm")
                    }

                    return IT_MOVE_SUCCESS;
                }

                // printf("HIT PORTAL!\n");

                // *newPos = collider->position;
                // *newVelocity = x_vec3_origin();
                // return 0;

                // Vec3fp posTemp = MakeVec3fp(*newPos);

                // *newPos = MakeVec3(portal->transformPointToOtherSide(posTemp));

                // Vec3fp velocityTemp = MakeVec3fp(collider->velocity);
                
                // Mat4x4 mat = portal->transformToOtherSide;
                // mat.dropTranslation();
                // mat.transpose3x3();

                // Vec3fp newV = mat.transform(velocityTemp);

                // *newVelocity = MakeVec3(newV);

                // collider->collisionInfo.type = BOXCOLLIDER_COLLISION_PORTAL;
                // collider->collisionInfo.hitPortal = portal;

                // moveUpUntilInLevel(level, newPos);

                // return IT_MOVE_SUCCESS;
            }
        }

        break;
    }
    
    IterationFlags flags = (IterationFlags)0;
    if(plane_is_vertical(&trace->collisionPlane))
        flags = (IterationFlags)(flags | IT_HIT_VERTICAL_WALL);
    
    if(plane_is_floor_surface(&trace->collisionPlane))
        flags = (IterationFlags)(flags | IT_ON_FLOOR);

    
    adjust_velocity_to_slide_along_wall(newVelocity, &trace->collisionPlane, collider->bounceCoefficient);
    
    *newPos = MakeVec3fp(trace->collisionPoint) + *newVelocity;
    
    return flags;
}

static int run_move_iterations(X_BoxCollider* collider, X_BspLevel* level, X_RayTracer* lastHitWall)
{
    Vec3fp newPos = collider->position + collider->velocity;
    Vec3fp newVelocity = collider->velocity;
    int flags = 0;
    
    const int MAX_ITERATIONS = 4;
    int i;
    for(i = 0; i < MAX_ITERATIONS && !(flags & IT_MOVE_SUCCESS); ++i)
        flags |= move_and_adjust_velocity(collider, level, lastHitWall, &newVelocity, &newPos);
    
    collider->position = newPos;
    collider->velocity = newVelocity;
    
    return flags | (i != 0 ? IT_HIT_ANYTHING : 0);
}

static bool try_and_move_up_step(X_BoxCollider* collider, X_BspLevel* level, Vec3fp* oldPos, Vec3fp* oldVelocity, X_RayTracer* lastHitWallFromFirstAttempt)
{
    Vec3fp adjustedPos = collider->position;
    Vec3fp adjustedVelocity = collider->velocity;
    
    const fp MAX_STEP_SIZE = fp::fromFloat(18.0);
    
    // Try and move the collider up the height of the step and reattempt the move
    collider->position = *oldPos;
    collider->position.y -= MAX_STEP_SIZE;
    
    collider->velocity = *oldVelocity;
    
    X_RayTracer lastHitWall;
    int flags = run_move_iterations(collider, level, &lastHitWall);
    bool successfullyClimbedStep = (flags & IT_MOVE_SUCCESS)
        && try_push_into_ground(collider, level, MAX_STEP_SIZE - fp::fromFloat(1), &lastHitWall)
        && plane_is_floor_surface(&lastHitWall.collisionPlane);
    
    if(!successfullyClimbedStep)
    {
        collider->position = adjustedPos;
        collider->velocity = adjustedVelocity;
        return 0;
    }
    
    // We successfully moved up the step, but we don't know how big the step was, so use the hit point
    // with the floor to reposition the collider
    collider->position = MakeVec3fp(lastHitWall.collisionPoint);
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
    
    collider->position = MakeVec3fp(floor.collisionPoint);
    adjust_velocity_to_slide_along_wall(&collider->velocity, &floor.collisionPlane, collider->bounceCoefficient);
    link_to_model_standing_on(collider, floor.hitModel);
    
    return IT_ON_FLOOR;
}

static bool try_move(X_BoxCollider* collider, X_BspLevel* level)
{
    Vec3fp oldVelocity = collider->velocity;
    Vec3fp oldPos = collider->position;
    
    X_RayTracer lastHitWall;
    int flags = run_move_iterations(collider, level, &lastHitWall);

    if(flags & IT_HIT_PORTAL)
    {
        oldPos = collider->position;
        oldVelocity = collider->velocity;

        printf("Hit portal!!!!!!\n");
    }
    
    if((flags & IT_HIT_VERTICAL_WALL) || (flags & IT_HIT_PORTAL))
    {
        if(try_and_move_up_step(collider, level, &oldPos, &oldVelocity, &lastHitWall))
        {
            printf("Moved up step\n");
            flags |= IT_MOVE_SUCCESS;
        }
    }
    
    if(!(flags & IT_MOVE_SUCCESS))
    {
        collider->velocity = Vec3fp(0, 0, 0);
        collider->position = oldPos;
        return 0;
    }
    
    flags |= stick_to_floor(collider, level);
    
    if(flags & IT_ON_FLOOR)
    {
        apply_friction(collider);
        collider->flags.set(X_BOXCOLLIDER_ON_GROUND);
    }
    else
    {
        collider->flags.reset(X_BOXCOLLIDER_ON_GROUND);
    }
    
    return 1;
}

void x_boxcollider_init(X_BoxCollider* collider, BoundBox* boundBox, EnumBitSet<X_BoxColliderFlags> flags)
{
    static Vec3fp gravity = { 0, fp::fromFloat(0.25), 0 };
    
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

#include "engine/X_Engine.h"

void x_boxcollider_update(X_BoxCollider* collider, X_BspLevel* level)
{
    // if(frameByFrame)
    // {
    //     auto key = x_engine_get_context()->getKeyState();

    //     bool down = x_keystate_key_down(key, (X_Key)'\n');

    //     if(down && lastDown)
    //     {
    //         return;
    //     }

    //     lastDown = down;

    //     if(!down)
    //     {
    //         return;
    //     }
    // }

    bool ran = false;

    for(Portal* portal = level->portalHead; portal != nullptr; portal = portal->next)
    {
        ran = true;
        collider->flags.set(BOXCOLLIDER_IN_PORTAL);
            collider->currentPortal = portal;
            break;
    }

    if(!ran)
    {
        return;
    }
    
    collider->collisionInfo.type = BOXCOLLIDER_COLLISION_NONE;
    
    unlink_from_model_standing_on(collider);
    apply_gravity(collider);
    try_move(collider, level);
}

