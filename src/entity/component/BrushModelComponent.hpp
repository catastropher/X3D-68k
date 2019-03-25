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

#include "geo/Vec3.hpp"
#include "system/Time.hpp"

class BspModel;
struct BspLevel;
struct X_Edict;
class Entity;
class EntityBuilder;

namespace internal
{
    using BrushModelReachedDestinationHandler = void (*)(Entity* entity);

    struct BrushModelMovement
    {
        Vec3fp direction;
        Vec3fp finalPosition;
        Time endTime;
        bool isMoving = false;

        BrushModelReachedDestinationHandler onArriveHandler;
    };

    class BrushModel
    {
    public:
        BrushModel(const EntityBuilder& builder);

        void initiateMoveTo(const Vec3fp &destination, Duration moveLength, BrushModelReachedDestinationHandler onArrive = nullptr);

        BspModel* model;
        BrushModelMovement movement;
    };
}

using BrushModelComponent = internal::BrushModel;

