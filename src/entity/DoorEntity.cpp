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

#include "DoorEntity.hpp"
#include "EntityDictionary.hpp"
#include "level/BspModel.hpp"
#include "engine/Engine.hpp"

static bool allowOpen = false;

static void door(X_EngineContext* engineContext, int argc, char* argv[])
{
    allowOpen = true;
}


DoorEntity::DoorEntity(X_Edict &edict, BspLevel &level)
    : Entity(level)
{
    addComponent<BrushModelComponent>(edict, level);
    x_console_register_cmd(Engine::getInstance()->getConsole(), "door", door);

    int angle;
    edict.getValueOrDefault("angle", angle, -1);

    if(angle == -1)
    {
        openDirection = Vec3fp(0, fp::fromInt(-1), 0);
    }
    else if(angle == -2)
    {
        openDirection = Vec3fp(0, fp::fromInt(1), 0);
    }
    else
    {
        fp openAngle = fp::fromInt(angle * 256 / 360) - fp(X_ANG_90);

        openDirection.x = x_cos(openAngle);
        openDirection.y = 0;
        openDirection.z = x_sin(openAngle);
    }
}

void DoorEntity::linkDoors(Array<DoorEntity*>& doorsInLevel)
{
    printf("Total doors in level: %d\n", doorsInLevel.count);

    int overlapCount = 0;

    for(int i = 0; i < doorsInLevel.count; ++i)
    {
        for(int j = i + 1; j < doorsInLevel.count; ++j)
        {
            auto a = doorsInLevel[i]->getComponent<BrushModelComponent>();
            auto b = doorsInLevel[j]->getComponent<BrushModelComponent>();

            if(a->model->boundBox.overlapsWith(b->model->boundBox))
            {
                ++overlapCount;
            }
        }
    }

    printf("Overlap count: %d\n", overlapCount);
}

void DoorEntity::update(const EntityUpdate& update)
{
    if(allowOpen)
    {
        auto transform = getComponent<TransformComponent>();

        transform->setPosition(transform->getPosition() + openDirection);
    }

    setNextUpdateTime(update.currentTime + Duration::fromMilliseconds(10));
}
