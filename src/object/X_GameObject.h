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

typedef struct X_GraphicsComponent
{
    
} X_GraphicsComponent;

typedef struct X_PhysicsComponent
{
    
} X_PhysicsComponent;

typedef struct X_InputComponent
{
    
} X_InputComponent;

struct X_GameObject;

typedef struct X_GameObjectEventHandlers
{
    void (*create_handler)(struct X_GameObject* obj);
    void (*destroy_handler)(struct X_GameObject* obj);
} X_GameObjectEventHandlers;

typedef struct X_GameObject
{
    int id;
    X_GraphicsComponent* graphics_component;
    X_PhysicsComponent* physics_component;
    X_InputComponent* input_component;
    
    X_Vec3 position;
    X_Vec3 velocity;
} X_GameObject;

struct X_EngineContext;

X_GameObject* x_gameobject_new(struct X_EngineContext* context, size_t objectSize);

