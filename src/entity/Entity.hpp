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

#include "Component.hpp"
#include "TransformComponent.hpp"
#include "system/Time.hpp"

struct BspLevel;

class Entity
{
public:
    Entity(const Entity&) = delete;
    
    void operator=(const Entity&) = delete;
    
    int getId() const
    {
        return id;
    }
    
    template<typename TComponent>
    TComponent* getComponent()
    {
        return componentManager.getComponent<TComponent>();
    }
    
    template<typename TComponent>
    int getComponentId()
    {
        return componentManager.getComponentId<TComponent>();
    }
    
    virtual void update(X_Time currentTime)
    {
        
    }

    virtual ~Entity()
    {

    }
    
protected:
    explicit Entity(BspLevel& level_)
        : id(-1),
        level(level_),
        nextUpdate(0)
    {
        addComponent<TransformComponent>();
    }
    
    template<typename TComponent, typename ...Args>
    TComponent* addComponent(Args&&... args)
    {
        TComponent* component =  componentManager.add<TComponent>(std::forward<Args>(args)...);
        
        component->owner = this;
        
        return component;
    }
    
    BspLevel& getLevel()
    {
        return level;
    }
    
    void setNextUpdateTime(X_Time nextUpdate)
    {
        this->nextUpdate = nextUpdate;
    }
    
private:    
    int id;
    ComponentManager componentManager;
    BspLevel& level;
    X_Time nextUpdate;
    
    friend class EntityManager;
};

