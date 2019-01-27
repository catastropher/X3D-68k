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

#include "EntityManager.hpp"
#include "EntityDictionary.hpp"
#include "WorldEntity.hpp"
#include "PlatformEntity.hpp"
#include "EntityDictionaryParser.hpp"

Entity* EntityManager::createEntityFromEdict(X_Edict& edict, BspLevel& level)
{
    Entity* entity = tryCreateEntity(edict, level);
    if(!entity)
    {
        return nullptr;
    }

    registerEntity(entity);

    return entity;
}

Entity* EntityManager::tryCreateEntity(X_Edict &edict, BspLevel &level)
{
    X_EdictAttribute* classname = edict.getAttribute("classname");
    if(classname == nullptr)
    {
        x_system_error("Edict missing classname");
    }

    if(strcmp(classname->value, "worldspawn") == 0)
    {
        return new WorldEntity(edict, level);
    }

    if(strcmp(classname->value, "func_plat") == 0)
    {
        return new PlatformEntity(edict, level);
    }

    if(createEntityCallback != nullptr)
    {
        return createEntityCallback(classname->value, edict, level);
    }

    if(strcmp(classname->value, "info_player_start") == 0)
    {
        x_system_error("Game does not implement entity info_player_start");
    }

    Log::error("Failed to create entity of type %s", classname->value);

    return nullptr;
}

void EntityManager::createEntitesInLevel(BspLevel& level)
{
    const char* nextEntry = level.entityDictionary;
    char valueData[4096];
    X_Edict edict;

    while((nextEntry = parseEdict(nextEntry, valueData, &edict)) != nullptr)
    {
        createEntityFromEdict(edict, level);
    }
}

void EntityManager::updateEntities(X_Time currentTime)
{
    for(Entity* entity : entities)
    {
        if(entity != nullptr && currentTime >= entity->nextUpdate)
        {
            entity->update(currentTime);
        }
    }
}

void EntityManager::destroyEntity(Entity* entity)
{
    // TODO: method to check if an entity is currently registered
    if(entity->id != -1)
    {
        entities[entity->id] = nullptr;

        while(entities.size() != 0 && entities[entities.size() - 1] == nullptr)
        {
            entities.pop_back();
        }
    }

    delete entity;
}

void EntityManager::destroyAllEntities()
{
    for(int i = 0; i < (int)entities.size(); ++i)
    {
        if(entities[i] != nullptr)
        {
            destroyEntity(entities[i]);
        }
    }
}

