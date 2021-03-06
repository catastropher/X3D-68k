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

#include "level/BspRayTracer.hpp"
#include "geo/Vec3.hpp"
#include "BoxColliderMoveLogic.hpp"
#include "entity/component/TransformComponent.hpp"
#include "entity/component/BoxColliderComponent.hpp"
#include "entity/EntityManager.hpp"

class BoxColliderMoveLogic;

struct BoxColliderState
{
    BoxColliderState(Vec3fp& position_, Vec3fp& velocity_)
        : position(position_),
        velocity(velocity_),
        flags()
    {
        newPosition = position + velocity;
    }

    Vec3fp position;
    Vec3fp newPosition;
    Vec3fp velocity;
    Flags<IterationFlags> flags;
};

class BoxColliderEngine
{
public:
    BoxColliderEngine(Entity* entity_, BspLevel& level_, fp dt_, EntityManager& entityManager)
        : entity(entity_),
        collider(*entity_->getComponent<BoxColliderComponent>()),
        transformComponent(*entity_->getComponent<TransformComponent>()),
        level(level_),
        dt(dt_),
        entityManager(entityManager)
    {

    }

    void runStep();

private:
    void tryMove();

    void useResultsFromMoveLogic(BoxColliderMoveLogic& moveLogic);

    void resetCollisionState();

    void unlinkFromModelStandingOn();
    void linkToModelStandingOn(BspModel* model);

    void applyGravity();
    void applyFriction();

    Entity* entity;
    BoxColliderComponent& collider;
    TransformComponent& transformComponent;
    BspLevel& level;
    EntityManager& entityManager;

    Vec3fp finalPosition;
    Vec3fp finalVelocity;
    Flags<IterationFlags> moveFlags;
    fp dt;
};

