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

#include "PlatformEntity.hpp"
#include "EntityManager.hpp"
#include "BrushModelComponent.hpp"
#include "BoxColliderComponent.hpp"
#include "level/BspRayTracer.hpp"
#include "physics/PhysicsEngine.hpp"

bool allowMove = false;

static void plat(EngineContext* engineContext, int argc, char* argv[])
{
    allowMove = true;
}

PlatformEntity::PlatformEntity(X_Edict& edict, BspLevel& level)
    : Entity(level)
{
    addComponent<BrushModelComponent>(edict, level);
    EngineContext* receiver = Engine::getInstance();
    x_console_register_cmd(receiver->console, "plat", plat);
}

void PlatformEntity::move(const Vec3fp &movement)
{

}

void PlatformEntity::handleEvent(EntityEvent& event)
{
    switch(event.type)
    {
        case CollideEntityEvent::Name:
            printf("I've been hit!\n");

            break;
    }
}

void PlatformEntity::update(const EntityUpdate& update)
{
    setNextUpdateTime(update.currentTime + Duration::fromMilliseconds(10));

    Vec3fp movement(0, fp::fromInt(-100) * update.deltaTime, 0);

    if(allowMove)
    {
        move(movement);
    }

}
