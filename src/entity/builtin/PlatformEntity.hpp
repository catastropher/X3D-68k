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
#include "entity/component/BrushModelComponent.hpp"

struct X_Edict;

class PlatformEntity : public Entity
{
public:
    PlatformEntity(X_Edict& edict, BspLevel& level);

    void update(const EntityUpdate& update);
    void move(const Vec3fp& movement);
    EntityEventResponse handleEvent(EntityEvent& event);
    
private:

    int height;
};

