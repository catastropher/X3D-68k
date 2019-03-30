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

#include <system/Clock.hpp>
#include "PhysicsComponent.hpp"
#include "entity/EntityDictionary.hpp"
#include "entity/EntityBuilder.hpp"

BrushModelPhysicsComponent::BrushModelPhysicsComponent(const EntityBuilder& builder)
    : PhysicsComponent(PhysicsComponentType::brushModel)
{
    const BrushModelId MISSING_MODEL(-1);

    BrushModelId id;
    builder.edict.getValueOrDefault("model", id, MISSING_MODEL);

    model = id.id != MISSING_MODEL.id
            ? x_bsplevel_get_model(builder.level, id.id)
            : nullptr;
}

void BrushModelPhysicsComponent::initiateMoveTo(const Vec3fp &destination, Duration moveLength, BrushModelReachedDestinationHandler onArrive)
{
    movement.endTime = Clock::getTicks() + moveLength;
    movement.finalPosition = destination;
    movement.direction = (destination - model->center) / moveLength.toSeconds();
    movement.onArriveHandler = onArrive;
    movement.isMoving = true;
}