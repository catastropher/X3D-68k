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

static void door(EngineContext* engineContext, int argc, char* argv[])
{
    allowOpen = true;
}


DoorEntity::DoorEntity(X_Edict &edict, BspLevel &level)
    : Entity(level)
{
    auto brushModel = addComponent<BrushModelComponent>(edict, level);
    EngineContext* receiver = Engine::getInstance();
    x_console_register_cmd(receiver->console, "door", door);

    int angle;
    edict.getValueOrDefault("angle", angle, -1);

    edict.print();

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

    BoundBox& box = brushModel->model->boundBox;
    Vec3 size = box.v[1] - box.v[0];

    Vec3fp bounds = Vec3fp(fp(size.x), fp(size.y), fp(size.z));

    openPosition = openDirection * abs(openDirection.dot(bounds));

    doorCloseCallback(this);
}



void DoorEntity::doorOpenCallback(Entity* entity)
{
    DoorEntity* door = static_cast<DoorEntity*>(entity);
    door->getComponent<BrushModelComponent>()->initiateMoveTo(Vec3fp(0, 0, 0), Duration::fromSeconds(fp::fromInt(5)), doorCloseCallback);
}

void DoorEntity::doorCloseCallback(Entity* entity)
{
    DoorEntity* door = static_cast<DoorEntity*>(entity);
    door->getComponent<BrushModelComponent>()->initiateMoveTo(door->openPosition, Duration::fromSeconds(fp::fromInt(5)), doorOpenCallback);
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
        if(closed)
        {
            closed = false;
            transitionTime = update.currentTime;
        }
        else
        {
            auto transform = getComponent<TransformComponent>();

            fp time = (Clock::getTicks() - transitionTime).toSeconds();

            if(time > fp::fromFloat(0.75))
            {
                return;
            }

            Ray3 ray(Vec3fp(0, 0, 0), openPosition);

            transform->setPosition(ray.lerp(time));
        }
    }

    setNextUpdateTime(update.currentTime + Duration::fromMilliseconds(10));
}
