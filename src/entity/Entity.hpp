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

#include "entity/component/Component.hpp"
#include "entity/component/TransformComponent.hpp"
#include "system/Time.hpp"
#include "EntityEvent.hpp"
#include "memory/DLink.hpp"

struct BspLevel;
class EngineContext;

enum class EntityFlags
{
    canPickThingsUp = (1 << 0),
    canBePickedUp = (1 << 1),
    canPush = (1 << 2),
    canBePushed = (1 << 3)
};

struct EntityUpdate
{
    EntityUpdate(Time currentTime_, fp deltaTime_, EngineContext* engineContext_)
        : currentTime(currentTime_),
        deltaTime(deltaTime_),
        engineContext(engineContext_)
    {

    }

    Time currentTime;
    fp deltaTime;
    EngineContext* engineContext;
};

class EntityBuilder;

struct EntityMetadata : DLink<EntityMetadata>
{
    StringId name;
    class Entity* (*buildCallback)(EntityBuilder& builder);
};

class Entity
{
public:
    Entity(const Entity&) = delete;
    
    void operator=(const Entity&) = delete;

    virtual ~Entity()
    {

    }

    template<typename T>
    T* getComponent()
    {
        static_assert(isValidComponentType<T>(), "Type is not a component");

        return componentRecord.getComponent<T>();
    }
    
protected:
    explicit Entity(BspLevel& level_)
        : id(-1),
        level(level_)
    {

    }
    
    BspLevel& getLevel()
    {
        return level;
    }
    
    void setNextUpdateTime(Time nextUpdate)
    {
        this->nextUpdate = nextUpdate;
    }

    Flags<EntityFlags> flags;
    
private:    
    int id;
    ComponentRecord componentRecord;
    const EntityMetadata* entityMetadata;

    BspLevel& level;
    Time nextUpdate;
    
    friend class EntityManager;
};

