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

struct X_EngineContext;
struct X_RenderContext;

////////////////////////////////////////////////////////////////////////////////
/// 
typedef struct X_CameraObject
{
    X_GameObject base;
    X_Viewport viewport;
    X_Mat4x4 viewMatrix;
    x_angle256 angleX;
    x_angle256 angleY;
    struct X_CameraObject* nextInCameraList;
    X_BspLeaf* currentLeaf;
    X_BspLeaf* lastLeaf;
    unsigned char pvsForCurrentLeaf[256];//X_BSPFILE_PVS_SIZE];
} X_CameraObject;

X_CameraObject* x_cameraobject_new(struct X_EngineContext* context);
void x_cameraobject_update_view(X_CameraObject* cam);
void x_cameraobject_render(X_CameraObject* cam, struct X_RenderContext* renderContext);

