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

////////////////////////////////////////////////////////////////////////////////
/// Creates a new camera object.
/// @todo Determine how to best initialize a camera
////////////////////////////////////////////////////////////////////////////////
X_CameraObject* x_cameraobject_new(X_EngineContext* context)
{
    X_CameraObject* cam = (X_CameraObject*)x_gameobject_new(context, sizeof(X_CameraObject));
    
    cam->lastLeaf = NULL;
    
    return cam;
}

void x_cameraobject_update_view(X_CameraObject* cam)
{
    X_Mat4x4 xRotation;
    x_mat4x4_load_x_rotation(&xRotation, cam->angleX);
    
    X_Mat4x4 yRotation;
    x_mat4x4_load_y_rotation(&yRotation, cam->angleY);
    
    X_Mat4x4 rotation;
    x_mat4x4_mul(&xRotation, &yRotation, &rotation);    
    
    X_Mat4x4 translation;
    X_Vec3 negatedPosition = x_vec3_neg(&cam->base.position);
    x_mat4x4_load_translation(&translation, &negatedPosition);
    
    x_mat4x4_mul(&rotation, &translation, &cam->viewMatrix);
    
    X_Vec3 camPos = x_vec3_fp16x16_to_vec3(&cam->base.position);
    
    X_Vec3 forward, up, right;
    x_mat4x4_extract_view_vectors(&cam->viewMatrix, &forward, &right, &up);
    x_viewport_update_frustum(&cam->viewport, &camPos, &forward, &right, &up);
}

static void x_cameraobject_determine_current_bspleaf(X_CameraObject* cam, X_RenderContext* renderContext)
{
    X_Vec3 position = x_vec3_fp16x16_to_vec3(&cam->base.position);
    cam->currentLeaf = x_bsplevel_find_leaf_point_is_in(renderContext->level, &position);
}

static void x_cameraobject_load_pvs_for_current_leaf(X_CameraObject* cam, X_RenderContext* renderContext)
{
    if(cam->currentLeaf == cam->lastLeaf)
        return;
    
    cam->lastLeaf = cam->currentLeaf;
    
    x_bsplevel_decompress_pvs_for_leaf(renderContext->level, cam->currentLeaf, cam->pvsForCurrentLeaf);
}

static void draw_current_leaf_info(X_CameraObject* cam, X_RenderContext* renderContext)
{
    char str[128];
    sprintf(str, "Current Leaf: %d\nVisible leaves: %d\n", (int)(cam->currentLeaf - renderContext->level->leaves),
            x_bsplevel_count_visible_leaves(renderContext->level, cam->pvsForCurrentLeaf));
    
    x_canvas_draw_str(renderContext->canvas, str, &renderContext->engineContext->mainFont, x_vec2_make(0, 0));
}

void x_cameraobject_render(X_CameraObject* cam, X_RenderContext* renderContext)
{
    x_assert(renderContext != NULL, "No render context");
    x_assert(renderContext->engineContext != NULL, "No engine context in render context");
    
    if(!x_engine_level_is_loaded(renderContext->engineContext))
        return;
    
    int currentFrame = x_enginecontext_get_frame(renderContext->engineContext);
    
    x_cameraobject_determine_current_bspleaf(cam, renderContext);
    x_cameraobject_load_pvs_for_current_leaf(cam, renderContext);
    x_bsplevel_mark_visible_leaves_from_pvs(renderContext->level, cam->pvsForCurrentLeaf, currentFrame);
    draw_current_leaf_info(cam, renderContext);
    
    x_bsplevel_render_wireframe(renderContext->level, renderContext, renderContext->screen->palette->brightRed);
}


