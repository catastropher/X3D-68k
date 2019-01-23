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

#include "GameObject.hpp"
#include "level/BspLevel.hpp"

typedef struct X_WorldObject
{
    X_GameObject base;
    BspModel* model;
} X_WorldObject;

struct X_ObjectFactory;
struct X_Edict;
struct X_EngineContext;

X_GameObject* x_worldobject_new(struct X_EngineContext* engineContext, struct X_Edict* edict);
void x_worldobject_register_type(struct X_ObjectFactory* factory);

