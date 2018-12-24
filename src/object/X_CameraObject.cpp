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

#include "X_CameraObject.h"
#include "engine/X_Engine.h"
#include "render/X_RenderContext.h"
#include "error/X_error.h"

static X_GameObjectType g_cameraObjectType = 
{
    0,
    "camera",
    {
        .update = NULL
    }
};

void X_CameraObject::overrideBspLeaf(int leafId, BspLevel* level)
{
    currentLeaf = level->leaves + leafId;
    lastLeaf = nullptr;
    flags.set(CAMERA_OVERRIDE_PVS);
}

////////////////////////////////////////////////////////////////////////////////
/// Creates a new camera object.
/// @todo Determine how to best initialize a camera
////////////////////////////////////////////////////////////////////////////////
X_CameraObject* x_cameraobject_new(X_EngineContext* context)
{
    X_CameraObject* cam = (X_CameraObject*)x_gameobject_new(context, sizeof(X_CameraObject));
    
    cam->lastLeaf = NULL;
    cam->screenResizeCallback = NULL;
    cam->base.velocity = x_vec3_origin();
    cam->base.type = &g_cameraObjectType;
    cam->flags.clear();
    
    return cam;
}

void X_CameraObject::updateView()
{
    Mat4x4 xRotation;
    xRotation.loadXRotation(angleX);
    
    Mat4x4 yRotation;
    yRotation.loadYRotation(angleY);
    
    Mat4x4 rotation = xRotation * yRotation;
    
    Mat4x4 translation;
    
    translation.loadTranslation(-position);
    
    viewMatrix = rotation * translation;

    updateFrustum();
}

void X_CameraObject::updateFrustum()
{
    Vec3fp forward, up, right;
    viewMatrix.extractViewVectors(forward, right, up);

    viewport.updateFrustum(position, forward, right, up);
}

static void x_cameraobject_determine_current_bspleaf(X_CameraObject* cam, X_RenderContext* renderContext)
{
    cam->currentLeaf = renderContext->level->findLeafPointIsIn(cam->position);
}

static void x_cameraobject_load_pvs_for_current_leaf(X_CameraObject* cam, X_RenderContext* renderContext)
{
    if(cam->currentLeaf == cam->lastLeaf)
        return;
    
    cam->lastLeaf = cam->currentLeaf;
    
    renderContext->level->decompressPvsForLeaf(cam->currentLeaf, cam->pvsForCurrentLeaf);
}

void x_cameraobject_render(X_CameraObject* cam, X_RenderContext* renderContext)
{
    x_assert(renderContext != NULL, "No render context");
    x_assert(renderContext->engineContext != NULL, "No engine context in render context");
    
    int currentFrame = x_enginecontext_get_frame(renderContext->engineContext);
    
    if(!cam->flags.hasFlag(CAMERA_OVERRIDE_PVS))
    {
        x_cameraobject_determine_current_bspleaf(cam, renderContext);
    }
    
    x_cameraobject_load_pvs_for_current_leaf(cam, renderContext);
    renderContext->level->markVisibleLeavesFromPvs(cam->pvsForCurrentLeaf, currentFrame);
    
    renderContext->camPos = x_cameraobject_get_position(cam);
    renderContext->currentFrame = currentFrame;
    
    if(cam->currentLeaf != renderContext->level->leaves + 0 && !renderContext->renderer->wireframe)
    {
        x_bsplevel_render(renderContext->level, renderContext);
    }
    else
    {
        renderContext->level->renderWireframe(*renderContext, 5 * 16 - 1);
    }
}


