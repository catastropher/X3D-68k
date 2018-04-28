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

#include "X_PlatformObject.h"
#include "error/X_error.h"
#include "physics/X_BoxCollider.h"

static void transition_next_state(X_PlatformObject* platform)
{
    platform->state = (X_PlatformObjectState)((platform->state + 1) % 4);
}

static void set_state(X_PlatformObject* platform, X_PlatformObjectState state)
{
    platform->state = state;
}

void move_platform(X_PlatformObject* platform, x_fp16x16 deltaTime)
{
    X_Time time = x_enginecontext_get_time(platform->base.engineContext);
    x_fp16x16 offset = x_fp16x16_mul(platform->speed, deltaTime);
    
    if(platform->state == X_PLATFORMOBJECT_DOWN)
    {
        bool activate = 0;
        if(platform->mode == X_PLATFORMOBJECT_CYCLE && time >= platform->nextTransition)
            activate = 1;
        else if(platform->mode == X_PLATFORMOBJECT_TRIGGER && x_bspmodel_has_objects_standing_on(platform->model))
            activate = 1;
        
        if(activate)
            transition_next_state(platform);
        
        return;
    }
    
    if(platform->state == X_PLATFORMOBJECT_UP)
    {
        if(platform->mode == X_PLATFORMOBJECT_TRIGGER_ONLY)
            return;
        
        if(time >= platform->nextTransition)
            transition_next_state(platform);
        
        return;
    }
    
    x_fp16x16 pos = platform->model->origin.y;
    
    if(platform->state == X_PLATFORMOBJECT_LOWERING)
        pos += offset;
    else
        pos -= offset;
    
    if(pos < 0 || pos >= platform->raiseHeight)
    {
        pos = x_fp16x16_clamp(pos, 0, platform->raiseHeight);
        platform->state = (X_PlatformObjectState)((platform->state + 1) % 4);
        platform->nextTransition = time + platform->waitTime;
    }
    
    platform->model->origin.y = pos;
}

#include <stddef.h>

static void move_objects_on_platform(X_PlatformObject* platform, x_fp16x16 dY)
{
    X_BspModel* model = platform->model;
    for(X_Link* link = model->objectsOnModelHead.next; link != &model->objectsOnModelTail; link = link->next)
    {
        X_BoxCollider* collider = (X_BoxCollider*)((unsigned char*)link - offsetof(X_BoxCollider, objectsOnModel));
        
        collider->position.y += dY;        
    }
}

static void x_platformobject_update(X_GameObject* obj, x_fp16x16 deltaTime)
{
    X_PlatformObject* platform = (X_PlatformObject*)obj;
    
    x_fp16x16 oldY = platform->model->origin.y;
    move_platform(platform, deltaTime);
    move_objects_on_platform(platform, platform->model->origin.y - oldY);
}

static void x_platform_trigger(X_GameObject* obj, X_GameObject* trigger, X_GameObjectTriggerType type)
{
    X_PlatformObject* platform = (X_PlatformObject*)obj;
    
    if(type == X_GAMEOBJECT_TRIGGER_ACTIVATE)
    {
        set_state(platform, X_PLATFORMOBJECT_RAISING);
        return;
    }
    
    if(type == X_GAMEOBJECT_TRIGGER_DEACTIVATE)
    {
        if(platform->mode != X_PLATFORMOBJECT_TRIGGER_ONLY)
            return;
        
        set_state(platform, X_PLATFORMOBJECT_LOWERING);
    }
}

static X_GameObjectType g_platformObjectType = 
{
    1,
    "func_plat",
    .handlers = 
    {
        .update = x_platformobject_update,
        .createNew = x_platformobject_new,
        .trigger = x_platform_trigger
    }
};

void x_platformobject_register_type(X_ObjectFactory* factory)
{
    x_objectfactory_register_type(factory, &g_platformObjectType);
}

X_GameObject* x_platformobject_new(X_EngineContext* engineContext, X_Edict* edict)
{
    printf("Created platform object\n");
    
    X_PlatformObject* obj = (X_PlatformObject*)x_gameobject_new(engineContext, sizeof(X_PlatformObject));
    
    int modelId = x_edict_get_model_id(edict, "model");
    obj->model = x_bsplevel_get_model(&engineContext->currentLevel, modelId);
    
    x_fp16x16 modelHeight = x_bspmodel_height(obj->model);
    
    obj->base.type = &g_platformObjectType;
    obj->speed = x_fp16x16_from_int(64);
    obj->waitTime = 1000 * 2;
    obj->state = X_PLATFORMOBJECT_DOWN;
    obj->nextTransition = x_enginecontext_get_time(engineContext) + obj->waitTime;
    
    int mode;
    x_edict_get_int(edict, "mode", X_PLATFORMOBJECT_TRIGGER, &mode);
    obj->mode = (X_PlatformObjectMode)mode;
    
    x_edict_get_fp16x16(edict, "height", modelHeight, &obj->raiseHeight);
    obj->model->origin.y = obj->raiseHeight;
    
    x_gameobject_activate(&obj->base);
    
    return (X_GameObject*)obj;
}

