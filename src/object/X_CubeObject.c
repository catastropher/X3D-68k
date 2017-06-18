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

#include "X_CubeObject.h"
#include "engine/X_EngineContext.h"

static void update_orientation(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    X_Quaternion spinVelocity = x_quaternion_make(cube->angularVelocity.x, cube->angularVelocity.y, cube->angularVelocity.z, 0);
    X_Quaternion spinTemp = x_quaternion_scale(&cube->orientation, X_FP16x16_HALF);
    X_Quaternion spin;
    
    x_quaternion_mul(&spinTemp, &spinVelocity, &spin);
    spin = x_quaternion_scale(&spin, deltaTime);
    
    cube->orientation = x_quaternion_add(&cube->orientation, &spin);
    x_quaternion_normalize(&cube->orientation);
}

static void update_geometry(X_CubeObject* cube)
{
    x_cube_init(&cube->geometry, cube->size.x, cube->size.y, cube->size.z);
    
    X_Mat4x4 mat;
    x_quaternion_to_mat4x4(&cube->orientation, &mat);
    
    mat.elem[0][3] = cube->center.x;
    mat.elem[1][3] = cube->center.y;
    mat.elem[2][3] = cube->center.z;
    
    x_cube_transform(&cube->geometry, &cube->geometry, &mat);
}

X_CubeObject* x_cubeobject_new(X_EngineContext* context, X_Vec3 pos, int width, int height, int depth)
{
    X_CubeObject* cube = (X_CubeObject*)x_gameobject_new(context, sizeof(X_CubeObject));
    
    cube->center = x_vec3_make(pos.x << 16, pos.y << 16, pos.z << 16);
    cube->size = x_vec3_make(width, height, depth);
    
    cube->angularVelocity = x_vec3_make(0, 0, 0);
    cube->linearVelocity = x_vec3_make(0, 0, 0);
    cube->orientation = x_quaternion_identity();
    
    update_geometry(cube);
    
    return cube;
}

void x_cubeobject_update_position(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    cube->center = x_vec3_add_scaled(&cube->center, &cube->linearVelocity, deltaTime);
    
    update_orientation(cube, deltaTime);
}

void x_cubeobject_update(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    x_cubeobject_update_position(cube, deltaTime);
    update_geometry(cube);
}

void x_cubeobject_render(X_CubeObject* cube, X_RenderContext* rcontext, X_Color color)
{
    x_cube_render(&cube->geometry, rcontext, color);
}
