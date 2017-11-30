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
#include "render/X_Viewport.h"
#include "level/X_BspLevel.h"
#include "physics/X_BoxCollider.h"

struct X_EngineContext;
struct X_RenderContext;
struct X_Screen;

////////////////////////////////////////////////////////////////////////////////
/// 
typedef struct X_CameraObject
{
    X_GameObject base;
    X_BoxCollider collider;
    X_Viewport viewport;
    X_Mat4x4 viewMatrix;
    x_fp16x16 angleX;
    x_fp16x16 angleY;
    struct X_CameraObject* nextInCameraList;
    X_BspLeaf* currentLeaf;
    X_BspLeaf* lastLeaf;
    unsigned char pvsForCurrentLeaf[256];//X_BSPFILE_PVS_SIZE];
    
    void (*screenResizeCallback)(struct X_CameraObject* cam, struct X_Screen* screen, x_fp16x16 fov);
} X_CameraObject;

X_CameraObject* x_cameraobject_new(struct X_EngineContext* context);
void x_cameraobject_update_view(X_CameraObject* cam);
void x_cameraobject_render(X_CameraObject* cam, struct X_RenderContext* renderContext);

static inline X_Vec3_fp16x16 x_cameraobject_get_position(X_CameraObject* cam)
{
    return cam->collider.position;
}

static inline X_Vec3_fp16x16 x_cameraobject_get_velocity(X_CameraObject* cam)
{
    return cam->collider.velocity;
}

static inline void x_cameraobject_add_angle(X_CameraObject* cam, X_Vec2_fp16x16 angleOffset)
{
    cam->angleX += angleOffset.x;
    cam->angleY += angleOffset.y;
}


