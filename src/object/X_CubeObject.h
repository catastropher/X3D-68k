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

#include "X_GameObject.h"
#include "geo/X_Cube.h"

struct X_EngineContext;

typedef struct X_CubeObject
{
    X_GameObject base;
    
    X_Vec3_fp16x16 center;
    X_Vec3_fp16x16 linearVelocity;
    X_Vec3_fp16x16 angularVelocity;
    X_Quaternion orientation;
    
    X_Mat4x4 intertia;
    X_Mat4x4 inverseInertia;
    
    x_fp16x16 mass;
    x_fp16x16 invMass;
    
    X_Vec3 forwardVec;
    X_Vec3 rightVec;
    X_Vec3 upVec;
    
    X_Vec3 force;
    X_Vec3 torque;
    
    X_Cube geometry;
    X_Vec3 size;
} X_CubeObject;

X_CubeObject* x_cubeobject_new(struct X_EngineContext* context, X_Vec3 pos, int width, int height, int depth, int mass);

void x_cubeobject_update_position(X_CubeObject* cube, x_fp16x16 deltaTime);
void x_cubeobject_update(X_CubeObject* cube, x_fp16x16 deltaTime);
void x_cubeobject_render(X_CubeObject* cube, X_RenderContext* rcontext, X_Color color);
void x_cubeobject_apply_force(X_CubeObject* cube, X_Vec3_fp16x16 force);

