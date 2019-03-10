//
// Created by michael on 3/3/19.
//

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

    allocator.allocAll();

    constructComponentIfPresent<TransformComponent>();
    constructComponentIfPresent<BrushModelComponent>();
    constructComponentIfPresent<BoxColliderComponent>();
    constructComponentIfPresent<InputComponent>();
    constructComponentIfPresent<CameraComponent>();

    componentRecord.types = components;

    return entityBytes;
}

void EntityBuilder::setupEntity(Entity* entity)
{
    entity->level = level;
    entity->componentRecord = componentRecord;
}

