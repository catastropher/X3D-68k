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

#include <memory/Set.hpp>
#include <engine/GlobalConfiguration.hpp>
#include "entity/system/IEntitySystem.hpp"
#include "entity/component/Component.hpp"
#include "entity/Entity.hpp"

// TODO: deprecate
class BrushModelSystem : public IEntitySystem
{
public:
    using SetType = Set<Entity*, Configuration::ENTITIES_MAX>;

    void createEntity(Entity& entity)
    {
        if(entityHasBrushModel(entity))
        {
            entities.add(&entity);
        }
    }

    void destroyEntity(Entity& entity)
    {
        if(entityHasBrushModel(entity))
        {
            entities.remove(&entity);
        }
    }


    SetType& getAllEntities()
    {
        return entities;
    }

private:
    bool entityHasBrushModel(Entity& entity)
    {
        PhysicsComponent* physicsComponent = entity.getComponent<PhysicsComponent>();

        return physicsComponent != nullptr && physicsComponent->type == PhysicsComponentType::brushModel;
    }

    SetType entities;
};
