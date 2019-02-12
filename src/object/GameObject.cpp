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

#include "GameObject.hpp"
#include "engine/EngineContext.hpp"
#include "PlatformObject.hpp"
#include "WorldObject.hpp"
#include "ButtonObject.hpp"

X_GameObject* x_gameobject_new(EngineContext* context, size_t objectSize)
{
    int objectHandle;
    X_GameObject* newObject = (X_GameObject*)x_factory_alloc(&context->getGameObjectFactory()->objectFactory, objectSize, &objectHandle);
    newObject->id = objectHandle;
    newObject->engineContext = context;
    
    return newObject;
}

void x_gameobject_extract_view_vectors(const X_GameObject* obj, Vec3* forwardDest, Vec3* rightDest, Vec3* upDest)
{
    // Mat4x4 mat;
    // x_quaternion_to_mat4x4(&obj->orientation, &mat);

    // Vec3fp u, r, f;

    // mat.extractViewVectors(f, r, u);

    // *forwardDest = MakeVec3(f);
    // *upDest = MakeVec3(u);
    // *rightDest = MakeVec3(r);
}

void x_gameobject_activate(X_GameObject* obj)
{
    X_GameObject* head = &obj->engineContext->activeObjectHead;
    
    obj->nextActive = head->nextActive;
    obj->prevActive = head;
    
    head->nextActive->prevActive = obj;
    head->nextActive = obj;
}

void x_gameobject_register_default_types(X_ObjectFactory* factory)
{
    x_platformobject_register_type(factory);
    x_worldobject_register_type(factory);
    x_buttonobject_register_type(factory);
}

void x_gameobject_trigger(X_GameObject* triggerSource, const char* name, X_GameObjectTriggerType type)
{
    if(name[0] == '\0')
        return;
    
    EngineContext* engineContext = triggerSource->engineContext;
    for(X_GameObject* obj = engineContext->activeObjectHead.nextActive; obj != &engineContext->activeObjectTail; obj = obj->nextActive)
    {
        if(strcmp(obj->triggerName, name) == 0 && obj->type->handlers.trigger)
            obj->type->handlers.trigger(obj, triggerSource, type);
    }
}

