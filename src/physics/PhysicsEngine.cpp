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

#include "PhysicsEngine.hpp"
#include "entity/component/BoxColliderComponent.hpp"
#include "BoxColliderEngine.hpp"
#include "entity/Entity.hpp"
#include "system/Clock.hpp"

// FIXME
extern bool physics;

void PhysicsEngine::update(BspLevel& level, fp timeDelta)
{
    step(level, timeDelta);
    moveBrushModels(level, timeDelta);
}


void PhysicsEngine::step(BspLevel& level, fp dt)
{
// FIXME: 2-20-2019
#if false
    if(!physics)
    {
        return;
    }
    
    auto boxColliders = BoxColliderComponent::getAll();
    
    for(auto& collider : boxColliders)
    {
        // FIXME: should be a dependency
        EntityManager* entityManager = Engine::getInstance()->entityManager;

        BoxColliderEngine engine(collider, level, dt, *entityManager);
        engine.runStep();
    }
#endif
}

void PhysicsEngine::moveBrushModels(BspLevel& level, fp dt)
{
// FIXME: 2-20-2019
#if false
    auto brushModels = BrushModelComponent::getAll();
    Time currentTime = Clock::getTicks();

    for(auto& brushModel : brushModels)
    {
        auto& movement = brushModel.movement;
        if(!movement.isMoving)
        {
            continue;
        }

        auto transform = brushModel.owner->getComponent<TransformComponent>();
        Vec3fp pushVector;

        if(currentTime >= movement.endTime)
        {
            pushVector = movement.finalPosition - transform->getPosition();
            movement.isMoving = false;

            if(movement.onArriveHandler != nullptr)
            {
                movement.onArriveHandler(brushModel.owner);
            }
        }
        else
        {
            pushVector = movement.direction * dt;
        }

        pushBrushEntity(brushModel.owner, pushVector, level);
    }
#endif
}

void PhysicsEngine::pushBrushEntity(Entity* brushEntity, const Vec3fp& movement, BspLevel& level)
{
// FIXME: 2-20-2019
#if false
    auto pos = brushEntity->getComponent<TransformComponent>();

    auto boxCollider = BoxColliderComponent::getAll();
    for(auto& collider : boxCollider)
    {
        auto transform = collider.owner->getComponent<TransformComponent>();

        if(collider.standingOnEntity == brushEntity)
        {
            // Move the object along with us
            transform->setPosition(transform->getPosition() + movement);
        }
        else
        {
            // Try moving the object into us in the reverse direction that we're moving
            Vec3fp position = transform->getPosition();
            Ray3 ray(position, position - movement);

            // For now, we are using hull 0, which is wrong
            BspRayTracer tracer(ray, &level, 0);

            if(tracer.trace() && tracer.getCollision().entity == brushEntity)
            {
                Vec3fp newPosition = tracer.getCollision().location.point + movement;
                transform->setPosition(newPosition);

                collider.standingOnEntity = brushEntity;
            }
        }
    }

    pos->setPosition(pos->getPosition() + movement);

    brushEntity->getComponent<BrushModelComponent>()->model->center = pos->getPosition();
#endif
}

void PhysicsEngine::sendCollideEvent(Entity* a, Entity* b)
{
// FIXME: 2-20-2019
#if false
    CollideEntityEvent eventForA(b);
    CollideEntityEvent eventForB(a);

    a->handleEvent(eventForA);
    b->handleEvent(eventForB);
#endif
}

