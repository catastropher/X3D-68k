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
#include "system/Time.hpp"
#include "math/FixedPoint.hpp"

struct InputUpdate
{
    InputUpdate(class KeyState* keyState, Time currentTime, fp deltaTime)
        : keyState(keyState),
          currentTime(currentTime),
          deltaTime(deltaTime)
    {

    }

    class KeyState* keyState;
    Time currentTime;
    fp deltaTime;
};

class Entity;
class EntityBuilder;

using InputUpdateHandler = bool (*)(Entity* entity, const InputUpdate& update);

namespace internal
{
    struct InputComponent
    {
        InputComponent(const EntityBuilder& builder)
        {

        }

        void setHandler(InputUpdateHandler handler)
        {
            this->handler = handler;
        }

        InputUpdateHandler handler;
    };
}

using InputComponent = internal::InputComponent;

