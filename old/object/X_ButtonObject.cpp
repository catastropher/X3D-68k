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

#include "X_ButtonObject.h"

static void x_buttonobject_update(X_GameObject* obj, x_fp16x16 deltaTime);
X_GameObject* x_buttonobject_new(X_EngineContext* engineContext, X_Edict* edict);

static X_GameObjectType g_buttonObjectType = 
{
    1,
    "func_button",
    {
        .update = x_buttonobject_update,
        .createNew = x_buttonobject_new
    }
};

static bool is_down(X_ButtonObject* button)
{
    return button->model->origin.y == button->modelHeight;
}

static void x_buttonobject_update(X_GameObject* obj, x_fp16x16 deltaTime)
{
    X_ButtonObject* button = (X_ButtonObject*)obj;
    x_fp16x16 moveOffset = x_fp16x16_mul(button->speed, deltaTime);
    
    bool hasObjectOnButton = button->model->objectsOnModelHead.next != &button->model->objectsOnModelTail;
    
    bool wasDown = is_down(button);
    
    if(!hasObjectOnButton)
    {
        button->model->origin.y -= moveOffset;
        
        if(button->model->origin.y < 0)
        {
            button->model->origin.y = 0;
        }
    }
    else if(button->model->origin.y < button->modelHeight)
        button->model->origin.y = X_MIN(button->model->origin.y + moveOffset, button->modelHeight);
    
    bool isDown = is_down(button);
    
    if(wasDown && !isDown)
        x_gameobject_trigger(obj, button->trigger, X_GAMEOBJECT_TRIGGER_DEACTIVATE);
    else if(!wasDown && isDown)
        x_gameobject_trigger(obj, button->trigger, X_GAMEOBJECT_TRIGGER_ACTIVATE);
}

X_GameObject* x_buttonobject_new(X_EngineContext* engineContext, X_Edict* edict)
{
    X_ButtonObject* obj = (X_ButtonObject*)x_gameobject_new(engineContext, sizeof(X_ButtonObject));
    
    x_edict_get_int(edict, "angle", 0, &obj->pushAngle);
    obj->model = x_bsplevel_get_model(engineContext->getCurrentLevel(), x_edict_get_model_id(edict, "model"));
    obj->modelHeight = x_bspmodel_height(obj->model);
    obj->base.type = &g_buttonObjectType;
    
    obj->speed = x_fp16x16_from_int(32);
    
    x_edict_get_str(edict, "target", "", obj->trigger);
    
    x_gameobject_activate(&obj->base);
    
    printf("Trigger: %s\n", obj->trigger);
    
    return (X_GameObject*)obj;
}

void x_buttonobject_register_type(X_ObjectFactory* factory)
{
    x_objectfactory_register_type(factory, &g_buttonObjectType);
}

