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
    
    // FIXME: need actual bound box
    static X_BoundBox box;
    x_boxcollider_init(&cam->collider, &box, X_BOXCOLLIDER_APPLY_GRAVITY);
    
    return cam;
}

void x_cameraobject_update_view(X_CameraObject* cam)
{
    X_Mat4x4 xRotation;
    xRotation.loadXRotation(cam->angleX);
    
    X_Mat4x4 yRotation;
    yRotation.loadYRotation(cam->angleY);
    
    X_Mat4x4 rotation = xRotation * yRotation;
    
    X_Mat4x4 translation;
    Vec3 position = x_cameraobject_get_position(cam);
    Vec3 negatedPosition = x_vec3_neg(&position);

    Vec3fp negatedPositionTemp = MakeVec3fp(negatedPosition);

    translation.loadTranslation(negatedPositionTemp);
    
    cam->viewMatrix = rotation * translation;
    
    Vec3 forward, up, right;
    x_mat4x4_extract_view_vectors(&cam->viewMatrix, &forward, &right, &up);

    Vec3fp f = MakeVec3fp(forward);
    Vec3fp r = MakeVec3fp(right);
    Vec3fp u = MakeVec3fp(up);
    Vec3fp p = MakeVec3fp(position);

    cam->viewport.updateFrustum(p, f, r, u);
}

static void x_cameraobject_determine_current_bspleaf(X_CameraObject* cam, X_RenderContext* renderContext)
{
    Vec3 position = x_cameraobject_get_position(cam);
    cam->currentLeaf = x_bsplevel_find_leaf_point_is_in(renderContext->level, &position);
}

static void x_cameraobject_load_pvs_for_current_leaf(X_CameraObject* cam, X_RenderContext* renderContext)
{
    if(cam->currentLeaf == cam->lastLeaf)
        return;
    
    cam->lastLeaf = cam->currentLeaf;
    
    x_bsplevel_decompress_pvs_for_leaf(renderContext->level, cam->currentLeaf, cam->pvsForCurrentLeaf);
}

void x_cameraobject_render(X_CameraObject* cam, X_RenderContext* renderContext)
{
    x_assert(renderContext != NULL, "No render context");
    x_assert(renderContext->engineContext != NULL, "No engine context in render context");
    
    // FIXME: why is this drawn here???
    if(!x_engine_level_is_loaded(renderContext->engineContext))
    {
        renderContext->canvas->drawStr("No level loaded", *renderContext->engineContext->getMainFont(), { 0, 0});
        return;
    }
    
    int currentFrame = x_enginecontext_get_frame(renderContext->engineContext);
    
    x_cameraobject_determine_current_bspleaf(cam, renderContext);
    x_cameraobject_load_pvs_for_current_leaf(cam, renderContext);
    x_bsplevel_mark_visible_leaves_from_pvs(renderContext->level, cam->pvsForCurrentLeaf, currentFrame);
    
    renderContext->camPos = x_cameraobject_get_position(cam);
    renderContext->currentFrame = currentFrame;
    
    if(cam->currentLeaf != renderContext->level->leaves + 0 && !renderContext->renderer->wireframe)
        x_bsplevel_render(renderContext->level, renderContext);
    else
        x_bsplevel_render_wireframe(renderContext->level, renderContext, renderContext->screen->palette->brightRed);
}


