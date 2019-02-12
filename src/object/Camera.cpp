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

#include "Camera.hpp"
#include "engine/Engine.hpp"
#include "render/RenderContext.hpp"
#include "error/Error.hpp"
#include "render/WireframeLevelRenderer.hpp"

static X_GameObjectType g_cameraObjectType = 
{
    0,
    "camera",
    {
        .update = NULL
    }
};

void Camera::overrideBspLeaf(int leafId, BspLevel* level)
{
    currentLeaf = level->leaves + leafId;
    lastLeaf = nullptr;
    flags.set(CAMERA_OVERRIDE_PVS);
}

void Camera::updateFrustum()
{
    Vec3fp forward, up, right;
    viewMatrix.extractViewVectors(forward, right, up);

    viewport.updateFrustum(position, forward, right, up);
}

static void x_cameraobject_determine_current_bspleaf(Camera* cam, X_RenderContext* renderContext)
{
    cam->currentLeaf = renderContext->level->findLeafPointIsIn(cam->position);
}

static void x_cameraobject_load_pvs_for_current_leaf(Camera* cam, X_RenderContext* renderContext)
{
    if(cam->currentLeaf == cam->lastLeaf)
    {
        return;
    }
    
    cam->lastLeaf = cam->currentLeaf;
    
    renderContext->level->pvs.decompressPvsForLeaf(*cam->currentLeaf, cam->pvsForCurrentLeaf);
}

void x_cameraobject_render(Camera* cam, X_RenderContext* renderContext)
{
    x_assert(renderContext != NULL, "No render context");
    x_assert(renderContext->engineContext != NULL, "No engine context in render context");
    
    int currentFrame = x_enginecontext_get_frame(renderContext->engineContext);
    
    if(!cam->flags.hasFlag(CAMERA_OVERRIDE_PVS))
    {
        x_cameraobject_determine_current_bspleaf(cam, renderContext);
    }
    
    x_cameraobject_load_pvs_for_current_leaf(cam, renderContext);
    renderContext->level->pvs.markVisibleLeaves(cam->pvsForCurrentLeaf, currentFrame);
    
    renderContext->camPos = x_cameraobject_get_position(cam);
    renderContext->currentFrame = currentFrame;
    
    if(cam->currentLeaf != renderContext->level->leaves + 0 && !renderContext->renderer->wireframe)
    {
        x_bsplevel_render(renderContext->level, renderContext);
    }
    else
    {
        WireframeLevelRenderer wireFrameLevelRenderer(*renderContext, 5 * 16 - 1, 15);
        
        wireFrameLevelRenderer.render();
    }
}


