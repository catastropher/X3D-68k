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
// FIXME: 2-20-2019
#if false
        auto collider = addComponent<BoxColliderComponent>();
        auto input = addComponent<InputComponent>(handleKeys);



        Vec3fp position;
        if(!edict.getValueOrDefault("origin", position, Vec3fp(0, 0, 0)))
        {
            x_system_error("No player start");
        }

        position.y -= fp::fromInt(30);

        getTransform().setPosition(position);

        CameraComponent* cam = addComponent<CameraComponent>();

        cam->viewport.init(Vec2(0, 0), 640, 480, X_ANG_60);

        angleX = 0;
        angleY = 0;

        registerVars();

        //, {16,16,24} },
        Vec3i mins(-16, -32, -16);
        Vec3i maxs(16, 24, 16);

        auto transform = getTransform();
        transform.setBoundBox(BoundBoxTemplate<fp>(mins.toVec3<fp>(), maxs.toVec3<fp>()));

        flags.set(EntityFlags::canPickThingsUp);
#endif
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

    static bool handleKeys(Entity* entity, const InputUpdate& update);

    EntityEventResponse handleEvent(EntityEvent& event);
    
    fp angleX;
    fp angleY;
    
private:
};
