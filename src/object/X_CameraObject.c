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
#include "engine/X_EngineContext.h"

////////////////////////////////////////////////////////////////////////////////
/// Creates a new camera object.
/// @todo Determine how to best initialize a camera
////////////////////////////////////////////////////////////////////////////////
X_CameraObject* x_cameraobject_new(X_EngineContext* context)
{
    return (X_CameraObject*)x_gameobject_new(context, sizeof(X_CameraObject));
}

void x_cameraobject_update_view(X_CameraObject* cam)
{
    X_Mat4x4 xRotation;
    x_mat4x4_load_x_rotation(&xRotation, cam->angleX);
    
    X_Mat4x4 yRotation;
    x_mat4x4_load_y_rotation(&yRotation, cam->angleY);
    
    x_mat4x4_mul(&xRotation, &yRotation, &cam->viewMatrix);    
    
    cam->viewMatrix.elem[0][3] = -cam->base.position.x >> 16;
    cam->viewMatrix.elem[1][3] = -cam->base.position.y >> 16;
    cam->viewMatrix.elem[2][3] = -cam->base.position.z >> 16;
    
    X_Vec3 camPos = x_vec3_fp16x16_to_vec3(&cam->base.position);
    
    X_Vec3 forward, up, right;
    x_mat4x4_extract_view_vectors(&cam->viewMatrix, &forward, &right, &up);
    x_viewport_update_frustum(&cam->viewport, &camPos, &forward, &right, &up);
}

