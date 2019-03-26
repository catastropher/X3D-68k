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

#include <new>


#include "memory/GroupAllocator.hpp"
#include "EntityManager.hpp"
#include "EntityBuilder.hpp"

void* EntityBuilder::allocateEntity(int entitySize, Flags<ComponentType> components)
{
    GroupAllocator allocator;
    unsigned char* entityBytes;

    allocator.scheduleAlloc(entityBytes, entitySize);

    if(components.hasFlag(ComponentType::transform))
    {
        allocator.scheduleAlloc(componentRecord.transformComponent);
    }

    if(components.hasFlag(ComponentType::brushModel))
    {
        allocator.scheduleAlloc(componentRecord.brushModelComponent);
    }

    if(components.hasFlag(ComponentType::collider))
    {
        allocator.scheduleAlloc(componentRecord.boxColliderComponent);
    }

    if(components.hasFlag(ComponentType::input))
    {
        allocator.scheduleAlloc(componentRecord.inputComponent);
    }

    if(components.hasFlag(ComponentType::camera))
    {
        allocator.scheduleAlloc(componentRecord.cameraComponent);
    }

    if(components.hasFlag(ComponentType::scriptable))
    {
        allocator.scheduleAlloc(componentRecord.scriptableComponent);
    }

    AxisAlignedBoundingBoxPhysicsComponent* axisAlignedBoundingBoxPhysicsComponent = nullptr;

    if(components.hasFlag(ComponentType::physics))
    {
        switch(physicsComponentOptions.type)
        {
            case PhysicsComponentType::axisAlignedBoundingBox:
                allocator.scheduleAlloc(axisAlignedBoundingBoxPhysicsComponent);
                break;

            default:
                x_system_error("Invalid physics component type");
        }
    }

    allocator.allocAll();

    setupComponentIfPresent<TransformComponent>(edict);
    setupComponentIfPresent<BrushModelComponent>(*this);
    setupComponentIfPresent<BoxColliderComponent>();
    setupComponentIfPresent<InputComponent>(inputComponentOptions.inputUpdateHandler);
    setupComponentIfPresent<CameraComponent>();
    setupComponentIfPresent<ScriptableComponent>();

    if(components.hasFlag(ComponentType::physics))
    {
        switch(physicsComponentOptions.type)
        {
            case PhysicsComponentType::axisAlignedBoundingBox:
                new (axisAlignedBoundingBoxPhysicsComponent) AxisAlignedBoundingBoxPhysicsComponent();
                componentRecord.physicsComponent = axisAlignedBoundingBoxPhysicsComponent;
                break;

            default:
                x_system_error("Invalid physics component type");
        }
    }


    componentRecord.types = components;

    return entityBytes;
}

void EntityBuilder::setupEntity(Entity* entity)
{
    entity->level = level;
    entity->componentRecord = componentRecord;
}

