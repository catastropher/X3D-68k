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

#include "X_ObjectFactory.h"
#include "engine/X_EngineContext.h"
#include "X_GameObjectLoader.h"
#include "error/X_error.h"
#include "error/X_log.h"

void x_objectfactory_init(X_ObjectFactory* factory, X_EngineContext* engineContext)
{
    x_factory_init(&factory->objectFactory, 20, 5);
    
    factory->engineContext = engineContext;
    factory->typeHead = NULL;
}

void x_objectfactory_cleanup(X_ObjectFactory* factory)
{
    x_factory_cleanup(&factory->objectFactory);
}

void x_objectfactory_register_type(X_ObjectFactory* factory, X_GameObjectType* type)
{
    type->next = factory->typeHead;
    factory->typeHead = type;
}

X_GameObjectType* x_objectfactory_get_type_by_name(X_ObjectFactory* factory, const char* name)
{
    for(X_GameObjectType* type = factory->typeHead; type != NULL; type = type->next)
    {
        if(strcmp(type->name, name) == 0)
            return type;
    }
    
    return NULL;
}

X_GameObjectType* x_objectfactory_get_type_by_id(X_ObjectFactory* factory, int typeId)
{
    for(X_GameObjectType* type = factory->typeHead; type != NULL; type = type->next)
    {
        if(type->typeId == typeId)
            return type;
    }
    
    return NULL;
}

X_GameObject* x_objectfactory_create_object_from_edict(X_ObjectFactory* factory, X_Edict* edict)
{
    X_EdictAttribute* classname = x_edict_get_attribute(edict, "classname");
    if(!classname)
        x_system_error("Edict missing classname");
    
    X_GameObjectType* type = x_objectfactory_get_type_by_name(factory, classname->value);
    if(!type)
    {
        x_log("Warning: unknown object type %s; skipping object creation", classname->value);
        return NULL;
    }
    
    if(!type->handlers.createNew)
        x_system_error("Object type %s is missing createNew()", classname->value);
    
    X_GameObject* obj = type->handlers.createNew(factory->engineContext, edict);
    if(!obj)
    {
        x_log_error("Couldn't create object of type %s\n", classname->value);
        return NULL;
    }
    
    X_EdictAttribute* name = x_edict_get_attribute(edict, "targetname");
    if(!name)
        return obj;
    
    strcpy(obj->triggerName, name->value);
    return obj;
}

