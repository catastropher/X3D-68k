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

static void x_platformobject_update(X_GameObject* obj, x_fp16x16 deltaTime)
{
    X_PlatformObject* platform = (X_PlatformObject*)obj;
    X_Time time = x_enginecontext_get_time(obj->engineContext);
 
    if(platform->state == X_PLATFORMOBJECT_DOWN || platform->state == X_PLATFORMOBJECT_UP)
    {
        if(time >= platform->nextTransition)
        {
            platform->state = (platform->state + 1) % 4;
            platform->transitionStart = time;
        }
        
        return;
    }
    
    x_fp16x16 pos;
    x_fp16x16 offset = x_fp16x16_from_int(x_fp16x16_mul(time - platform->transitionStart, platform->speed) / 1000);
    
    if(platform->state == X_PLATFORMOBJECT_LOWERING)
        pos = offset;
    else
        pos = platform->raiseHeight - offset;
    
    if(pos < 0 || pos >= platform->raiseHeight)
    {
        pos = x_fp16x16_clamp(pos, 0, platform->raiseHeight);
        platform->state = (platform->state + 1) % 4;
        platform->nextTransition = time + platform->waitTime;
    }
    
    platform->model->origin.y = pos;
    
}

static X_GameObjectType g_platformObjectType = 
{
    .typeId = 1,
    .name = "func_plat",
    .handlers = 
    {
        .update = x_platformobject_update,
        .createNew = x_platformobject_new
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
    
    x_edict_get_fp16x16(edict, "height", modelHeight, &obj->raiseHeight);
    
    //obj->model->origin.y = -obj->raiseHeight;
    
    x_gameobject_activate(&obj->base);
    
    return (X_GameObject*)obj;
}

