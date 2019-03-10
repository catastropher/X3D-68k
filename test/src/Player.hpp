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

#include <X3D/X3D.hpp>

enum class PlayerKeys
{
    forward = 1,
    backward = 2,
    strafeLeft = 4,
    strafeRight = 8,
    jump = 16,
    lookUp = 32,
    lookDown = 64,
    lookLeft = 128,
    lookRight = 256
};

using PlayerKeyFlags = Flags<PlayerKeys>;

class Player : public Entity
{
public:
    Player()
    {

    }

    void registerVars();
    
    BoxColliderComponent& getCollider()
    {
        return *getComponent<BoxColliderComponent>();
    }
    
    TransformComponent& getTransform()
    {
        return *getComponent<TransformComponent>();
    }

    void handleMovement(const InputUpdate& update);

    static Entity* build(EntityBuilder& builder);
    static bool handleKeys(Entity* entity, const InputUpdate& update);

    EntityEventResponse handleEvent(EntityEvent& event);
    
    fp angleX;
    fp angleY;
    
private:
};
