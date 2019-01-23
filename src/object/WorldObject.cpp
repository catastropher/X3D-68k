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

#include "WorldObject.hpp"
#include "engine/EngineContext.hpp"
#include "GameObjectLoader.hpp"

static X_GameObjectType g_worldObjectType = 
{
    .typeId = 2,
    "worldspawn",
    .handlers = 
    {
        .update = NULL,
        .createNew = x_worldobject_new
    }
};

X_GameObject* x_worldobject_new(X_EngineContext* engineContext, X_Edict* edict)
{
    X_WorldObject* obj = (X_WorldObject*)x_gameobject_new(engineContext, sizeof(X_WorldObject));
    
    obj->model = x_bsplevel_get_level_model(engineContext->getCurrentLevel());
    
    return (X_GameObject*)obj;
}

void x_worldobject_register_type(X_ObjectFactory* factory)
{
    x_objectfactory_register_type(factory, &g_worldObjectType);
}

