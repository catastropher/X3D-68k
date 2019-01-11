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


#include "PlayerMoveLogic.hpp"

static Vec3fp removeVerticalComponent(Vec3fp v)
{
    Vec3fp withoutVertical = Vec3fp(v.x, 0, v.z);
    withoutVertical.normalize();
    
    return withoutVertical;
}

void PlayerMoveLogic::applyMovement(PlayerKeyFlags keys, BspLevel* level)
{
    if(!enablePhysics)
    {
        player.getCollider().flags.reset(X_BOXCOLLIDER_APPLY_GRAVITY);
        player.getCollider().velocity = Vec3fp(0, 0, 0);
        
        Vec3fp impulseVelocity;
        Vec3fp frameVelocity;
        
        getMovementVector(keys, impulseVelocity, frameVelocity);
        
        player.getTransform().setPosition(player.getTransform().getPosition() + frameVelocity);
    }
    else
    {
        player.getCollider().flags.set(X_BOXCOLLIDER_APPLY_GRAVITY);
    }
    
    handleAngleKeys(keys);
    handleNormalMovement(keys, level);
}

void PlayerMoveLogic::getMovementVector(PlayerKeyFlags keys, Vec3fp& impulseVelocityDest, Vec3fp& frameVelocityDest)
{
    if(!x_boxcollider_is_on_ground(&player.getCollider()) && enablePhysics)
    {
        impulseVelocityDest = Vec3fp(0, 0, 0);
        frameVelocityDest = Vec3fp(0, 0, 0);
        
        return;
    }
    
    frameVelocityDest = getMovementKeyVector(keys, enablePhysics);
    impulseVelocityDest = getJumpVector(keys);   
}

Vec3fp PlayerMoveLogic::getMovementKeyVector(PlayerKeyFlags keys, bool ignoreVerticalComponent)
{
    fp forwardComponent = getForwardComponent(keys);
    fp rightComponent = getStrafeComponent(keys);
    
    Vec3fp forward, right, up;
    
    // TODO: should not use camera
    player.camera.viewMatrix.extractViewVectors(forward, right, up);
    
    if(ignoreVerticalComponent)
    {
        forward = removeVerticalComponent(forward);
        right = removeVerticalComponent(right);
    }
    
    Vec3fp totalVelocity = forward * forwardComponent
        + right * rightComponent;
    
    
    if(totalVelocity.x == 0 && totalVelocity.y == 0 && totalVelocity.z == 0)
        return totalVelocity;
    
    totalVelocity.normalize();
    
    return totalVelocity * moveSpeed;
}

fp PlayerMoveLogic::getForwardComponent(PlayerKeyFlags keys)
{
    if(keys.hasFlag(PlayerKeys::forward))
    {
        return fp::fromInt(1);
    }
    
    if(keys.hasFlag(PlayerKeys::backward))
    {
        return fp::fromInt(-1);
    }
    
    return 0;
}

fp PlayerMoveLogic::getStrafeComponent(PlayerKeyFlags keys)
{
    if(keys.hasFlag(PlayerKeys::strafeLeft))
    {
        return fp::fromInt(-1);
    }
    
    if(keys.hasFlag(PlayerKeys::strafeRight))
    {
        return fp::fromInt(1);
    }
    
    return 0;
}

Vec3fp PlayerMoveLogic::getJumpVector(PlayerKeyFlags keys)
{
    if(!enablePhysics)
    {
        return Vec3fp(0, 0, 0);
    }
    
    // TODO: this should be configurable
    fp jumpVelocity = fp::fromFloat(-6.0);
    
    if(keys.hasFlag(PlayerKeys::jump))
    {
        return Vec3fp(0, jumpVelocity, 0);
    }
    
    // TODO: should there be an origin method?
    return Vec3fp(0, 0, 0);
}

void PlayerMoveLogic::handleAngleKeys(PlayerKeyFlags keys)
{
    // TODO: configure
    fp dAngle = fp::fromFloat(2.0);
    
    if(keys.hasFlag(PlayerKeys::lookUp))
    {
        player.angleX -= dAngle;
    }
    else if(keys.hasFlag(PlayerKeys::lookDown))
    {
        player.angleX += dAngle;
    }
    
    if(keys.hasFlag(PlayerKeys::lookLeft))
    {
        player.angleY += dAngle;
    }
    else if(keys.hasFlag(PlayerKeys::lookRight))
    {
        player.angleY -= dAngle;
    }
}

bool handle_no_collision_keys(X_EngineContext* engineContext, X_CameraObject* cam, X_KeyState* keyState)
{
    return false;
    
//     if(x_engine_level_is_loaded(engineContext) && enablePhysics)
//     {
//         Vec3fp camPos = MakeVec3fp(x_cameraobject_get_position(cam));
//         
//         if(engineContext->getCurrentLevel()->findLeafPointIsIn(camPos)->contents != X_BSPLEAF_SOLID)
//             return 0;
//     }
//     
//     Vec3 movementVector = get_movement_key_vector(cam, keyState, 0);
//     cam->collider.position += MakeVec3fp(movementVector);
//     cam->updateView();
//     
//     return 1;
}

void PlayerMoveLogic::handleNormalMovement(PlayerKeyFlags keys, BspLevel* level)
{
    Vec3fp impulseVelocity;
    Vec3fp frameVelocity;
    
    getMovementVector(keys, impulseVelocity, frameVelocity);
    
    auto& collider = player.getCollider();
    
    collider.applyFrameVelocity(frameVelocity);
    collider.applyImpulseVelocity(impulseVelocity);
    
#if false
    if(cam->collider.collisionInfo.type == BOXCOLLIDER_COLLISION_PORTAL)
    {
        auto portal = cam->collider.collisionInfo.hitPortal;
        
        cam->angleX += portal->transformAngleX.toFp16x16();
        cam->angleY += portal->transformAngleY.toFp16x16();
        
        printf("Cam hit portal!\n");
    }
#endif
}


