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

#include <stdio.h>

#include "geo/X_Vec3.h"
#include "math/X_Quaternion.h"
#include "memory/X_Link.h"

struct X_GameObject;
struct X_EngineContext;
struct X_Edict;

typedef enum X_GameObjectTriggerType
{
    X_GAMEOBJECT_TRIGGER_ACTIVATE,
    X_GAMEOBJECT_TRIGGER_DEACTIVATE
} X_GameObjectTriggerType;

typedef struct X_GameObjectEventHandlers
{
    void (*update)(struct X_GameObject* obj, x_fp16x16 deltaTime);
    struct X_GameObject* (*createNew)(struct X_EngineContext* engineContext, struct X_Edict* edict);
    void (*trigger)(struct X_GameObject* obj, struct X_GameObject* trigger, X_GameObjectTriggerType type);
} X_GameObjectEventHandlers;

typedef struct X_GameObjectType
{
    int typeId;
    char name[32];
    X_GameObjectEventHandlers handlers;
    
    struct X_GameObjectType* next;
} X_GameObjectType;

typedef struct X_GameObject
{
    char triggerName[32];
    int id;    
    Vec3 position;
    Vec3 velocity;
    X_Quaternion orientation;
    X_GameObjectType* type;
    
    struct X_GameObject* nextActive;
    struct X_GameObject* prevActive;
    
    struct X_EngineContext* engineContext;
} X_GameObject;


class GameObject
{
    
};

struct X_EngineContext;
struct X_ObjectFactory;

void x_gameobject_register_default_types(struct X_ObjectFactory* factory);

X_GameObject* x_gameobject_new(struct X_EngineContext* context, size_t objectSize);
void x_gameobject_extract_view_vectors(const X_GameObject* obj, Vec3* forwardDest, Vec3* rightDest, Vec3* upDest);

void x_gameobject_activate(X_GameObject* obj);
void x_gameobject_trigger(X_GameObject* triggerSource, const char* name, X_GameObjectTriggerType type);

