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

#include <geo/Vec3.hpp>
#include <system/Time.hpp>
#include <entity/EntityEvent.hpp>
#include "memory/BitSet.hpp"

class Entity;
class EntityBuilder;
class BspModel;

enum class PhysicsComponentType
{
    axisAlignedBoundingBox,
    brushModel
};

enum class PhysicsComponentFlags
{
    isTrigger = 1
};

class PhysicsComponent
{
public:
    PhysicsComponent(PhysicsComponentType type_)
        : type(type_)
    {

    }

    PhysicsComponentType type;
    Flags<PhysicsComponentFlags> flags;
};

class AxisAlignedBoundingBoxPhysicsComponent : public PhysicsComponent
{
public:
    AxisAlignedBoundingBoxPhysicsComponent()
        : PhysicsComponent(PhysicsComponentType::axisAlignedBoundingBox)
    {

    }
};

using BrushModelReachedDestinationHandler = void (*)(Entity* entity);

struct BrushModelMovement
{
    Vec3fp direction;
    Vec3fp finalPosition;
    Time endTime;
    bool isMoving = false;

    BrushModelReachedDestinationHandler onArriveHandler;
};

class BrushModelPhysicsComponent : public PhysicsComponent
{
public:
    BrushModelPhysicsComponent(const EntityBuilder& entityBuilder);

    void initiateMoveTo(const Vec3fp &destination, Duration moveLength, BrushModelReachedDestinationHandler onArrive = nullptr);

    BspModel* model;
    BrushModelMovement movement;
};

class TriggerEntityEvent : public EntityEvent
{
public:
    static constexpr StringId Name = StringId("trigger");

    TriggerEntityEvent(Entity* hitEntity_)
        : EntityEvent(Name),
        hitEntity(hitEntity_)
    {

    }

    Entity* hitEntity;
};

