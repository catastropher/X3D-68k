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

#include "X_GameObjectLoader.h"
#include "X_GameObject.h"
#include "engine/X_EngineContext.h"

typedef enum X_PlatformObjectState
{
    X_PLATFORMOBJECT_DOWN,
    X_PLATFORMOBJECT_RAISING,
    X_PLATFORMOBJECT_UP,
    X_PLATFORMOBJECT_LOWERING,
    X_PLATFORMOBJECT_WAITING
} X_PlatformObjectState;

typedef enum X_PlatformObjectMode
{
    X_PLATFORMOBJECT_CYCLE = 0,
    X_PLATFORMOBJECT_TRIGGER = 1,
    X_PLATFORMOBJECT_TRIGGER_ONLY = 2
} X_PlatformObjectMode;

typedef struct X_PlatformObject
{
    X_GameObject base;
    BspModel* model;
    x_fp16x16 raiseHeight;
    x_fp16x16 speed;
    X_Time nextTransition;
    X_Time waitTime;
    X_PlatformObjectState state;
    X_PlatformObjectMode mode;
} X_PlatformObject;

X_GameObject* x_platformobject_new(X_EngineContext* engineContext, X_Edict* edict);
void x_platformobject_register_type(X_ObjectFactory* factory);

