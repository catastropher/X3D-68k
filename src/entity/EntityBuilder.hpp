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

class EntityBuilder
{
public:
    EntityBuilder(BspLevel* level_, X_Edict& edict_)
        : edict(edict_),
        level(level_)
    {

    }

    template<typename TComponent>
    EntityBuilder& withComponent()
    {
        static_assert(isValidComponentType<TComponent>(), "Not a valid component type");

        components.set(getComponentType<TComponent>());

        return *this;
    }

    EntityBuilder& withInputComponent(InputUpdateHandler inputUpdateHandler)
    {
        components.set(getComponentType<InputComponent>());

        inputComponentOptions.inputUpdateHandler = inputUpdateHandler;

        return *this;
    }

    template<typename TEntity, typename ...TConstructorArgs>
    TEntity* build(TConstructorArgs&&... args)
    {
        TEntity* entity = (TEntity*)allocateEntity(sizeof(TEntity), components);

        new (entity) TEntity(std::forward<TConstructorArgs>(args)...);

        setupEntity(entity);

        return entity;
    }

    void* allocateEntity(int entitySize, Flags<ComponentType> components);
    void setupEntity(Entity* entity);

    X_Edict& edict;
    BspLevel* level;

private:
    struct InputComponentOptions
    {
        InputUpdateHandler inputUpdateHandler;
    };

    template<typename TComponent, typename ...TConstructorArgs>
    void setupComponentIfPresent(TConstructorArgs&&... args)
    {
        if(components.hasFlag(getComponentType<TComponent>()))
        {
            new (componentRecord.getComponent<TComponent>()) TComponent(std::forward<TConstructorArgs>(args)...);
        }
    }

    Flags<ComponentType> components;
    ComponentRecord componentRecord;

    InputComponentOptions inputComponentOptions;
};