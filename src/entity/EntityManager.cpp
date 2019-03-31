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

#include "entity/builtin/DoorEntity.hpp"
#include "EntityManager.hpp"
#include "EntityDictionary.hpp"
#include "EntityDictionaryParser.hpp"
#include "builtin/WorldEntity.hpp"
#include "builtin/TriggerEntity.hpp"
#include "EntityBuilder.hpp"

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
// FIXME: 2-20-2019
    X_EdictAttribute* classname = edict.getAttribute("classname");
    if(classname == nullptr)
    {
        x_system_error("Edict missing classname");
    }

    StringId nameId = StringId::fromString(classname->value);

    for(EntityMetadata* metadata = entityMetadataHead; metadata != nullptr; metadata = metadata->next)
    {
        if(metadata->name == nameId)
        {
            EntityBuilder builder(&level, edict);
            Entity* entity = metadata->buildCallback(builder);

            for(int i = 0; i < entitySystems.size(); ++i)
            {
                entitySystems[i]->createEntity(*entity);
            }

            return entity;
        }
    }

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

void EntityManager::destroyEntity(Entity* entity)
{
    for(auto& entitySystem : entitySystems)
    {
        entitySystem->destroyEntity(*entity);
    }

    // TODO: method to check if an entity is currently registered
    if(entity->id != -1)
    {
        entities[entity->id] = nullptr;

        while(entities.size() != 0 && entities[entities.size() - 1] == nullptr)
        {
            entities.popBack();
        }
    }

    entity->~Entity();

    x_free(entity);
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

void EntityManager::registerBuiltinTypes()
{
    registerEntityType<WorldEntity>("worldspawn"_sid, WorldEntity::build);
    //registerEntityType<DoorEntity>("func_door"_sid, DoorEntity::build);
    registerEntityType<TriggerEntity>("trigger_once"_sid, TriggerEntity::build);
}

