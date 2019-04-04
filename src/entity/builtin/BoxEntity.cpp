// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
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

#include "BoxEntity.hpp"
#include "entity/EntityBuilder.hpp"
#include "level/BrushModelBuilder.hpp"
#include "level/BspLevelLoader.hpp"
#include "hud/MessageQueue.hpp"

Entity* BoxEntity::build(EntityBuilder& builder)
{
    BoxEntity* entity = builder
        .withComponent<TransformComponent>()
        .withComponent<ScriptableComponent>()
        .withComponent<BoxColliderComponent>()
        .withRenderComponent(RenderComponentType::quake)
        .build<BoxEntity>();

    TransformComponent* transformComponent = entity->getComponent<TransformComponent>();

    entity->getComponent<BoxColliderComponent>()->gravity->y = 0.1_fp;
    entity->getComponent<BoxColliderComponent>()->maxSpeed = 500.0_fp;
    entity->getComponent<BoxColliderComponent>()->bounceCoefficient = 1.0_fp;
    entity->getComponent<BoxColliderComponent>()->flags.set(X_BOXCOLLIDER_APPLY_GRAVITY);

    x_entitymodel_load_from_file(&entity->entityModel, "ogre.mdl");

    entity->getComponent<QuakeModelRenderComponent>()->model = &entity->entityModel;
    entity->getComponent<QuakeModelRenderComponent>()->playAnimation("walk", true);

    return entity;
}

EntityEventResponse BoxEntity::handleEvent(Entity& entity, const EntityEvent& event)
{
    switch(event.type)
    {
        case TriggerEntityEvent::Name:
            Engine::getInstance()->messageQueue->addMessage("Hit zer box");

            return EntityEventResponse::accepted;

        default:
            return EntityEventResponse::unhandled;
    }
}

