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
#include "EntityDictionaryParser.hpp"
#include "memory/GroupAllocator.hpp"
#include "builtin/WorldEntity.hpp"

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

    Log::info("search...");

    for(EntityMetadata* metadata = entityMetadataHead; metadata != nullptr; metadata = metadata->next)
    {
        if(metadata->name == nameId)
        {
            EntityBuilder builder(&level, edict);

            return metadata->buildCallback(builder);
        }
    }


#if false


    if(strcmp(classname->value, "worldspawn") == 0)
    {
        return new WorldEntity(edict, level);
    }

    if(strcmp(classname->value, "func_plat") == 0)
    {
        return new PlatformEntity(edict, level);
    }

    if(strcmp(classname->value, "func_door") == 0)
    {
        return new DoorEntity(edict, level);
    }

    if(createEntityCallback != nullptr)
    {
        Entity* entity = createEntityCallback(classname->value, edict, level);

        if(entity != nullptr)
        {
            return entity;
        }
    }

    if(strcmp(classname->value, "info_player_start") == 0)
    {
        x_system_error("Game does not implement entity info_player_start");
    }

    Log::error("Failed to create entity of type %s", classname->value);
#endif
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

void EntityManager::updateEntities(Time currentTime, fp deltaTime, EngineContext* engineContext)
{
    EntityUpdate update(currentTime, deltaTime, engineContext);

    for(Entity* entity : entities)
    {
        if(entity != nullptr && currentTime >= entity->nextUpdate)
        {
// FIXME: 2-20-2019
#if false
            entity->update(update);
#endif
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

void* EntityBuilder::allocateEntity(int entitySize, Flags<ComponentType> components)
{
    GroupAllocator allocator;
    unsigned char* entityBytes;

    allocator.scheduleAlloc(entityBytes, entitySize);

    if(components.hasFlag(ComponentType::transform))
    {
        allocator.scheduleAlloc(componentRecord.transformComponent);
    }

    if(components.hasFlag(ComponentType::brushModel))
    {
        allocator.scheduleAlloc(componentRecord.brushModelComponent);
    }

    if(components.hasFlag(ComponentType::collider))
    {
        allocator.scheduleAlloc(componentRecord.boxColliderComponent);
    }

    if(components.hasFlag(ComponentType::input))
    {
        allocator.scheduleAlloc(componentRecord.inputComponent);
    }

    if(components.hasFlag(ComponentType::camera))
    {
        allocator.scheduleAlloc(componentRecord.cameraComponent);
    }

    allocator.allocAll();

    constructComponentIfPresent<TransformComponent>();
    constructComponentIfPresent<BrushModelComponent>();
    constructComponentIfPresent<BoxColliderComponent>();
    constructComponentIfPresent<InputComponent>();
    constructComponentIfPresent<CameraComponent>();

    return entityBytes;
}

void EntityManager::registerBuiltinTypes()
{
    registerEntityType<WorldEntity>("worldspawn"_sid, WorldEntity::build);
}

