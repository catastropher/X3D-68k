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

#include <type_traits>

#include "memory/StringId.hpp"
#include "system/Time.hpp"

class EngineContext;

enum class EntityEventResponse
{
    unhandled = 0,
    accepted = 1,
    rejected = 2
};

struct EntityEvent
{
    constexpr EntityEvent(StringId typeName)
        : type(typeName)
    {

    }

    template<typename T>
    T* to() const
    {
        static_assert(std::is_base_of<EntityEvent, T>::value);

        return static_cast<T*>(this);
    }

    const StringId type;
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

