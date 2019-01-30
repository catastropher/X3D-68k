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

using PlayerKeyFlags = EnumBitSet<PlayerKeys>;

class Player : public Entity
{
public:
    Player(X_Edict& edict, BspLevel& level)
        : Entity(level)
    {
        auto collider = addComponent<BoxColliderComponent>();
        auto input = addComponent<InputComponent>(handleKeys);



        Vec3fp position;
        edict.getValueOrDefault("origin", position, Vec3fp(0, 0, 0));

        position.y -= fp::fromInt(30);

        getTransform().setPosition(position);

        CameraComponent* cam = addComponent<CameraComponent>();

        cam->viewport.init(Vec2(0, 0), 640, 480, X_ANG_60);

        angleX = 0;
        angleY = 0;
    }
    
    BoxColliderComponent& getCollider()
    {
        return *getComponent<BoxColliderComponent>();
    }
    
    TransformComponent& getTransform()
    {
        return *getComponent<TransformComponent>();
    }

    void handleMovement(const InputUpdate& update);

    static bool handleKeys(Entity* entity, const InputUpdate& update);
    
    fp angleX;
    fp angleY;
    
private:
};
