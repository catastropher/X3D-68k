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

#include "Entity.hpp"
#include "BrushModelComponent.hpp"

class DoorEntity : public Entity
{
public:
    DoorEntity(X_Edict& edict, BspLevel& level);

    void update(const EntityUpdate& update);

    static void linkDoors(Array<DoorEntity*>& doorsInLevel);

private:
    static void doorOpenCallback(Entity* entity);
    static void doorCloseCallback(Entity* entity);

    Vec3fp openDirection;
    Vec3fp openPosition;
    Time transitionTime;
    bool closed = true;
};

