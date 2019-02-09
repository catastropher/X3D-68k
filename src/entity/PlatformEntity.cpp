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

#include "level/BrushModelBuilder.hpp"

bool allowMove = false;

static void plat(X_EngineContext* engineContext, int argc, char* argv[])
{
    allowMove = true;
}

PlatformEntity::PlatformEntity(X_Edict& edict, BspLevel& level)
    : Entity(level)
{
    addComponent<BrushModelComponent>(edict, level);
    x_console_register_cmd(Engine::getInstance()->getConsole(), "plat", plat);
}

void PlatformEntity::move(const Vec3fp &movement)
{
    auto pos = getComponent<TransformComponent>();

    auto boxCollider = BoxColliderComponent::getAll();
    for(auto& collider : boxCollider)
    {
        auto transform = collider.owner->getComponent<TransformComponent>();

        if(collider.standingOnEntity == static_cast<Entity*>(this))
        {
            // Move the object along with us

            transform->setPosition(transform->getPosition() + movement);
        }
        else
        {
            // Try moving the object into us in the reverse direction that we're moving
            Vec3fp position = transform->getPosition();
            Ray3 ray(position, position - movement);

            // For now, we are using hull 0, which is wrong
            BspRayTracer tracer(ray, &getLevel(), 0);

            if(tracer.trace() && tracer.getCollision().entity == this)
            {
                Vec3fp newPosition = tracer.getCollision().location.point + movement;
                transform->setPosition(newPosition);

                collider.standingOnEntity = this;
            }
        }
    }

    pos->setPosition(pos->getPosition() + movement);

    getComponent<BrushModelComponent>()->model->center = pos->getPosition();
}

void PlatformEntity::update(const EntityUpdate& update)
{
    setNextUpdateTime(update.currentTime + 10);

    Vec3fp movement(0, fp::fromInt(-5000) * update.deltaTime, 0);

    if(allowMove)
    {
        move(movement);
    }

}
