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

#include "IEntitySystem.hpp"
#include "engine/GlobalConfiguration.hpp"
#include "memory/Set.hpp"
#include "entity/Entity.hpp"

template<typename TComponent, int MaxEntities>
class GenericComponentSystem : public IEntitySystem
{
public:
    using SetType = Set<Entity*, MaxEntities>;

    void createEntity(Entity& entity)
    {
        if(entity.hasComponent<TComponent>())
        {
            entities.add(&entity);
        }
    }

    void destroyEntity(Entity& entity)
    {
        if(entity.hasComponent<TComponent>())
        {
            entities.remove(&entity);
        }
    }


    SetType& getAllEntities()
    {
        return entities;
    }

private:
    SetType entities;
};
