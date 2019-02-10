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

#include <vector>
#include <type_traits>

#include "Entity.hpp"
#include "engine/Engine.hpp"

struct X_Edict;

using CreateEntityCallback = Entity* (*)(const char* name, X_Edict& edict, BspLevel& level);

class EntityManager
{
public:
    EntityManager()
        : createEntityCallback(nullptr)
    {

    }

    void registerEntity(Entity* entity)
    {
        entities.push_back(entity);
        entity->id = entities.size() - 1;
    }

    void unregisterEntity(Entity* entity)
    {
        if(entity->id == -1)
        {
            return;
        }

        entities[entity->id] = nullptr;
        entity->id = -1;
    }

    void setCreateEntityCallback(CreateEntityCallback createEntityCallback)
    {
        this->createEntityCallback = createEntityCallback;
    }
    
    void updateEntities(Time currentTime, fp deltaTime, X_EngineContext* engineContext);
    
    Entity* createEntityFromEdict(X_Edict& edict, BspLevel& level);
    void createEntitesInLevel(BspLevel& level);

    void destroyEntity(Entity* entity);
    void destroyAllEntities();
    
private:
    Entity* tryCreateEntity(X_Edict& edict, BspLevel& level);

    std::vector<Entity*> entities;
    CreateEntityCallback createEntityCallback;
};

