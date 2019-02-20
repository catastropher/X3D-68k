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

#include <new>

#include "Entity.hpp"
#include "engine/Engine.hpp"

struct X_Edict;

using CreateEntityCallback = Entity* (*)(const char* name, X_Edict& edict, BspLevel& level);

class EntityManager
{
public:
    EntityManager()
        : entityMetadataHead(nullptr)
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
    
    void updateEntities(Time currentTime, fp deltaTime, EngineContext* engineContext);
    
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

    template<typename T>
    void registerEntityType(StringId name, class Entity* (*buildCallback)(EntityBuilder& builder))
    {
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
            entityMetadata.insertAfterThis(entityMetadataHead);
        }
    }

private:
    Entity* tryCreateEntity(X_Edict& edict, BspLevel& level);

    template<typename TEntity>
    struct EntityMetadataProvider
    {
        static EntityMetadata entityMetadata;
    };

    EntityMetadata* entityMetadataHead;
    std::vector<Entity*> entities;
    CreateEntityCallback createEntityCallback;
};

template<typename T>
EntityMetadata EntityManager::EntityMetadataProvider<T>::entityMetadata;

class EntityBuilder
{
public:
    template<typename TComponent>
    EntityBuilder& withComponent()
    {
        static_assert(isValidComponentType<TComponent>(), "Not a valid component type");

        components.set(getComponentType<TComponent>());

        return *this;
    }

    template<typename TEntity, typename ...TConstructorArgs>
    TEntity* build(TConstructorArgs&&... args)
    {
        TEntity* entity = allocateEntity(sizeof(TEntity), components);

        return new (entity) TEntity(std::forward<TConstructorArgs>(args)...);
    }

    void* allocateEntity(int entitySize, Flags<ComponentType> components);

private:
    Flags<ComponentType> components;
    ComponentRecord componentRecord;
};

