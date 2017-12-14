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

struct X_GameObject;
struct X_EngineContext;

typedef struct X_GameObjectEventHandlers
{
    void (*onStep)(struct X_EngineContext* context, struct X_GameObject* obj, x_fp16x16 deltaTime);
} X_GameObjectEventHandlers;

typedef struct X_GameObjectType
{
    int typeId;
    char name[16];
    X_GameObjectEventHandlers handlers;
} X_GameObjectType;

typedef struct X_GameObject
{
    int id;    
    X_Vec3_fp16x16 position;
    X_Vec3_fp16x16 velocity;
    X_Quaternion orientation;
    X_GameObjectType* type;
} X_GameObject;

struct X_EngineContext;

X_GameObject* x_gameobject_new(struct X_EngineContext* context, size_t objectSize);
void x_gameobject_extract_view_vectors(const X_GameObject* obj, X_Vec3_fp16x16* forwardDest, X_Vec3_fp16x16* rightDest, X_Vec3_fp16x16* upDest);

