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

#include "Entity.hpp"
#include "engine/GlobalConfiguration.hpp"
#include "system/IEntitySystem.hpp"
#include "memory/FixedSizeArray.hpp"

struct X_Edict;

using CreateEntityCallback = Entity* (*)(const char* name, X_Edict& edict, BspLevel& level);

class EntityManager
{
public:
    EntityManager()
        : entityMetadataHead(nullptr)
    {
        registerBuiltinTypes();
    }

    void registerEntity(Entity* entity)
    {
        entities.pushBack(entity);

        entity->id = entities.size() - 1;
        printf("New id: %d\n", entity->id);
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

    void registerEntitySystem(IEntitySystem* entitySystem)
    {
        entitySystems.pushBack(entitySystem);
    }

    Entity* createEntityFromEdict(X_Edict& edict, BspLevel& level);
    void createEntitesInLevel(BspLevel& level);

    void destroyEntity(Entity* entity);
    void destroyAllEntities();

    template<typename T>
    void getAllEntitiesOfType(Array<T*>& outArray)
    {
        int count = 0;
        for(Entity* entity : entities)
        {
            T* e;
            if((e = dynamic_cast<T*>(entity)) != nullptr)
            {
                outArray[count++] = e;
            }
        }

        outArray.count = count;
    }

    const FixedLengthArray<Entity*, Configuration::ENTITIES_MAX>& getAllEntities()
    {
        return entities;
    }

    template<typename T>
    void registerEntityType(StringId name, class Entity* (*buildCallback)(EntityBuilder& builder))
    {
        if(buildCallback == nullptr)
        {
            x_system_error("Null entity builder callback");
        }

        EntityMetadata& entityMetadata = EntityMetadataProvider<T>::entityMetadata;

        entityMetadata.name = name;
        entityMetadata.buildCallback = buildCallback;

        // TODO: put in linked list class
        if(entityMetadataHead == nullptr)
        {
            entityMetadataHead = &entityMetadata;
        }
        else
        {
            entityMetadata.next = entityMetadataHead;
            entityMetadataHead = &entityMetadata;
        }
    }

private:
    void registerBuiltinTypes();

    Entity* tryCreateEntity(X_Edict& edict, BspLevel& level);

    template<typename TEntity>
    struct EntityMetadataProvider
    {
        static EntityMetadata entityMetadata;
    };

    EntityMetadata* entityMetadataHead;
    FixedLengthArray<Entity*, Configuration::ENTITIES_MAX> entities;
    FixedLengthArray<IEntitySystem*, Configuration::ENTITY_MAX_SYSTEMS> entitySystems;
};

template<typename T>
EntityMetadata EntityManager::EntityMetadataProvider<T>::entityMetadata;

