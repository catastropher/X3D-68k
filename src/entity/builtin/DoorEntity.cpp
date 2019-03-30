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
#include "entity/EntityDictionary.hpp"
#include "level/BspModel.hpp"
#include "engine/Engine.hpp"
#include "entity/EntityBuilder.hpp"

static bool allowOpen = false;

static void door(EngineContext* engineContext, int argc, char* argv[])
{
    allowOpen = true;
}


DoorEntity::DoorEntity(X_Edict &edict)
{

}



void DoorEntity::doorOpenCallback(Entity* entity)
{
    DoorEntity* door = static_cast<DoorEntity*>(entity);
    door->getComponent<BrushModelPhysicsComponent>()->initiateMoveTo(Vec3fp(0, 0, 0), Duration::fromSeconds(fp::fromInt(5)), doorCloseCallback);
}

void DoorEntity::doorCloseCallback(Entity* entity)
{
    DoorEntity* door = static_cast<DoorEntity*>(entity);
    door->getComponent<BrushModelPhysicsComponent>()->initiateMoveTo(door->openPosition, Duration::fromSeconds(fp::fromInt(5)), doorOpenCallback);
}

void DoorEntity::linkDoors(Array<DoorEntity*>& doorsInLevel)
{
    printf("Total doors in level: %d\n", doorsInLevel.count);

    int overlapCount = 0;

    for(int i = 0; i < doorsInLevel.count; ++i)
    {
        for(int j = i + 1; j < doorsInLevel.count; ++j)
        {
            auto a = doorsInLevel[i]->getComponent<BrushModelPhysicsComponent>();
            auto b = doorsInLevel[j]->getComponent<BrushModelPhysicsComponent>();

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

#if false
    setNextUpdateTime(update.currentTime + Duration::fromMilliseconds(10));
#endif
}

Entity* DoorEntity::build(EntityBuilder& builder)
{
    DoorEntity* doorEntity = builder
        .withComponent<TransformComponent>()
        .withComponent<ScriptableComponent>()
        .build<DoorEntity>(builder.edict);

    EngineContext* receiver = Engine::getInstance();
    x_console_register_cmd(receiver->console, "door", door);

    int angle;
    builder.edict.getValueOrDefault("angle", angle, -1);

    builder.edict.print();

    if(angle == -1)
    {
        doorEntity->openDirection = Vec3fp(0, fp::fromInt(-1), 0);
    }
    else if(angle == -2)
    {
        doorEntity->openDirection = Vec3fp(0, fp::fromInt(1), 0);
    }
    else
    {
        fp openAngle = fp::fromInt(angle * 256 / 360) - fp(X_ANG_90);

        doorEntity->openDirection.x = x_cos(openAngle);
        doorEntity->openDirection.y = 0;
        doorEntity->openDirection.z = x_sin(openAngle);
    }

    auto brushModelComponent = doorEntity->getComponent<BrushModelPhysicsComponent>();

    printf("Has model: %d\n", brushModelComponent->model != nullptr);

    BoundBox& box = brushModelComponent->model->boundBox;
    Vec3 size = box.v[1] - box.v[0];

    Vec3fp bounds = Vec3fp(fp(size.x), fp(size.y), fp(size.z));

    doorEntity->openPosition = doorEntity->openDirection * abs(doorEntity->openDirection.dot(bounds));

    doorCloseCallback(doorEntity);

    return doorEntity;
}


