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

#define X_BUTTONOBJECT_PUSH_UP -1
#define X_BUTTONOBJECT_PUSH_DOWN -2

typedef enum X_ButtonObjectState
{
    X_BUTTONOBJECT_DOWN,
    X_BUTTONOBJECT_LOWERING,
    X_BUTTONOBJECT_UP,
    X_BUTTONOBJECT_RAISING
} X_ButtonObjectState;

typedef struct X_ButtonObject
{
    X_GameObject base;
    X_BspModel* model;
    int pushAngle;
    x_fp16x16 modelHeight;
    x_fp16x16 speed;
    X_ButtonObjectState state;
    X_Time lastTransiitonTime;
} X_ButtonObject;

void x_buttonobject_register_type(X_ObjectFactory* factory);

