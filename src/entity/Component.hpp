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
#include <new>

#include "memory/X_BitSet.hpp"
#include "memory/X_Array.hpp"
#include "ComponentDefs.hpp"

enum class ComponentFlags
{
    inUse = 1
};

const int COMPONENT_INVALID_ID = -1;

struct ComponentHandle
{
    ComponentHandle()
    {
        
    }
    
    ComponentHandle(int id_, int typeId_)
        : id(id_),
        typeId(typeId_)
    {
        
    }
    
    int id;
    int typeId;
};

class Entity;

template<typename TComponent, int TypeId>
class Component : public TComponent
{
public:
    Entity* owner;
    
    static int add()
    {
        int id = components.size();
        
        components.push_back(Component());
        
        new (&components[id]) Component();
        
        return id;
    }
    
    static Component* getById(int id)
    {
        if(id < 0 || id >= components.size())
        {
            return nullptr;
        }
        
        return &components[id];
    }
    
    static Array<Component> getAll()
    {
        return Array<Component>(&components[0], components.size());
    }
    
    static const int TYPE_ID = TypeId;

private:
    static std::vector<Component> components;
};

template<typename TComponent, int TypeId>
std::vector<Component<TComponent, TypeId>> Component<TComponent, TypeId>::components;

class ComponentManager
{
public:
    ComponentManager()
        : totalHandles(0)
    {
        
    }
    
    template<typename TComponent>
    TComponent* add()
    {
        // TODO: check whether that component already exists
        handles[totalHandles] = createComponent<TComponent>();
        
        TComponent* component = TComponent::getById(handles[totalHandles].id);
        
        ++totalHandles;
        
        return component;
    }
    
    template<typename TComponent>
    TComponent* getComponent()
    {
        for(int i = 0; i < totalHandles; ++i)
        {
            if(handles[i].typeId == TComponent::TYPE_ID)
            {
                return TComponent::getById(handles[i].id);
            }
        }
        
        return nullptr;
    }
    
    template<typename TComponent>
    int getComponentId()
    {
        for(int i = 0; i < totalHandles; ++i)
        {
            if(handles[i].typeId == TComponent::TYPE_ID)
            {
                return handles[i].id;
            }
        }
        
        return COMPONENT_INVALID_ID;
    }
    
private:
    static const int MAX_COMPONENTS = 10;
    
    template<typename TComponent>
    static ComponentHandle createComponent()
    {
        int id = TComponent::add();
        
        return ComponentHandle(id, TComponent::TYPE_ID);
    }
    
    ComponentHandle handles[MAX_COMPONENTS];
    int totalHandles;
};

