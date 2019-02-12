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
#include "memory/Factory.h"

struct X_GameObjectType;
struct X_GameObject;
struct EngineContext;

typedef struct X_ObjectFactory
{
    struct EngineContext* engineContext;
    X_GameObjectType* typeHead;
    
    X_Factory objectFactory;
} X_ObjectFactory;

void x_objectfactory_init(X_ObjectFactory* factory, struct EngineContext* engineContext);
void x_objectfactory_cleanup(X_ObjectFactory* factory);

void x_objectfactory_register_type(X_ObjectFactory* factory, X_GameObjectType* type);
X_GameObjectType* x_objectfactory_get_type_by_name(X_ObjectFactory* factory, const char* name);
X_GameObjectType* x_objectfactory_get_type_by_id(X_ObjectFactory* factory, int typeId);
X_GameObject* x_objectfactory_create_object_from_edict(X_ObjectFactory* factory, struct X_Edict* edict);

