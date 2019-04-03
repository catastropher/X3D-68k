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
#include "memory/FixedLengthString.hpp"
#include "level/LevelManager.hpp"
#include "entity/builtin/BoxEntity.hpp"

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

    Log::error("No such entity type: %s", classname->value);

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

    registerEntityType<BoxEntity>("box"_sid, BoxEntity::build);
}

static bool splitKeyValuePair(const char* str, XString& outKey, XString& outValue)
{
    while(*str != ':' && *str != '\0')
    {
        outKey += *str++;
    }

    if(*str == '\0')
    {
        return false;
    }

    ++str;

    while(*str != '\0')
    {
        outValue += *str++;
    }

    return true;
}

void EntityManager::cmdEntitySpawn(EngineContext* engineContext, int argc, char** argv)
{
    X_Edict edict;
    edict.totalAttributes = argc - 1;

    XString keys[16];
    XString values[16];

    for(int i = 1; i < argc; ++i)
    {
        if(!splitKeyValuePair(argv[i], keys[i - 1], values[i - 1]))
        {
            x_console_printf(engineContext->console, "Invalid key value pair\n");
        }

        X_EdictAttribute* attribute = &edict.attributes[i - 1];

        strcpy(attribute->name, keys[i - 1].c_str());
        attribute->value = (char *)values[i - 1].c_str();
    }

    BspLevel* level = engineContext->levelManager->getCurrentLevel();

    engineContext->entityManager->createEntityFromEdict(edict, *level);

    edict.print();
}
