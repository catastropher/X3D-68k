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

#include "memory/BitSet.hpp"
#include "memory/Array.hpp"

enum class ComponentFlags
{
    inUse = 1
};

const int COMPONENT_INVALID_ID = -1;

struct ComponentType
{
    void (*deleteComponent)(int id);
};

struct ComponentHandle
{
    ComponentHandle()
    {
        
    }
    
    ComponentHandle(int id_, ComponentType* type_)
        : id(id_),
        type(type_)
    {
        
    }
    
    int id;
    ComponentType* type;
};

class Entity;

template<typename TComponent>
class Component : public TComponent
{
public:
    template<typename ...Args>
    Component(Args&&... args)
        : TComponent(std::forward<Args>(args)...)
    {

    }

    Entity* owner;

    template<typename ...Args>
    static Component* add(ComponentHandle& dest, Args&&... args)
    {
        int id = components.size();
        
        components.push_back(Component(std::forward<Args>(args)...));
        
        Component* component = &components[id];
        
        dest.id = id;
        dest.type = &type;
        
        return component;
    }
    
    static void deleteComponent(int id)
    {
        components[id].owner = nullptr;
    }
    
    static Component* getById(int id)
    {
        if(id < 0 || id >= (int)components.size())
        {
            return nullptr;
        }
        
        return &components[id];
    }
    
    static ComponentType* getType()
    {
        return &type;
    }

    static Array<Component> getAll()
    {
        return Array<Component>(&components[0], components.size());
    }

private:

    static std::vector<Component> components;
    static ComponentType type;
};

template<typename TComponent>
std::vector<Component<TComponent>> Component<TComponent>::components;

template<typename TComponent>
ComponentType Component<TComponent>::type = 
{
    deleteComponent
};

class ComponentManager
{
public:
    ComponentManager()
        : totalHandles(0)
    {
        
    }
    
    template<typename TComponent, typename ...Args>
    TComponent* add(Args&... args)
    {
        // TODO: check whether that component already exists
       return TComponent::add(handles[totalHandles++], std::forward<Args>(args)...);
    }
    
    template<typename TComponent>
    TComponent* getComponent()
    {
        ComponentType* type = TComponent::getType();
        
        for(int i = 0; i < totalHandles; ++i)
        {
            if(handles[i].type == type)
            {
                return TComponent::getById(handles[i].id);
            }
        }
        
        return nullptr;
    }
    
    template<typename TComponent>
    int getComponentId()
    {
        ComponentType* type = TComponent::getType();
        
        for(int i = 0; i < totalHandles; ++i)
        {
            if(handles[i].type == type)
            {
                return handles[i].id;
            }
        }
        
        return COMPONENT_INVALID_ID;
    }
    
    ~ComponentManager()
    {
        for(int i = 0; i < totalHandles; ++i)
        {
            if(handles[i].type != nullptr)
            {
                handles[i].type->deleteComponent(handles[i].id);
            }
        }
    }
    
private:
    static const int MAX_COMPONENTS = 10;
    
    ComponentHandle handles[MAX_COMPONENTS];
    int totalHandles;
};

