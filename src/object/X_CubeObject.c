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
#include "util/X_util.h"

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

static void reset_forces(X_CubeObject* cube)
{
    cube->torque = x_vec3_make(0, 0, 0);
    cube->force = x_vec3_make(0, 0, 0);
}

static void calculate_inverse_static_inertia(X_CubeObject* cube)
{
    int x = cube->size.x;
    int y = cube->size.y;
    int z = cube->size.z;
    
    int xx = (x * x);
    int yy = (y * y);
    int zz = (z * z);
    int massOverTwelve = cube->mass / 12;
    
    int inertiaX = (yy + zz) * massOverTwelve;
    int inertiaY = (xx + zz) * massOverTwelve;
    int inertiaZ = (xx + yy) * massOverTwelve;
    
    X_Mat4x4 inertia =
    {
        {
            { inertiaX, 0, 0, 0 },
            { 0, inertiaY, 0, 0 },
            { 0, 0, inertiaZ, 0 },
            { 0, 0, 0, X_FP16x16_ONE }
        }
    };
    
    x_mat4x4_invert_diagonal(&inertia, &cube->inverseStaticInertia);
    
    x_mat4x4_print(&inertia);
    x_mat4x4_print(&cube->inverseStaticInertia);
}

static void calculate_inverse_inertia(X_CubeObject* cube)
{
    X_Mat4x4 transform;
    x_quaternion_to_mat4x4(&cube->orientation, &transform);
    
    X_Mat4x4 temp;
    x_mat4x4_mul(&cube->inverseStaticInertia, &transform, &temp);
    
    x_mat4x4_transpose_3x3(&transform);
    
    x_mat4x4_mul(&transform, &temp, &cube->inverseInertia);    
}

X_CubeObject* x_cubeobject_new(X_EngineContext* context, X_Vec3 pos, int width, int height, int depth, int mass)
{
    X_CubeObject* cube = (X_CubeObject*)x_gameobject_new(context, sizeof(X_CubeObject));
    
    cube->center = x_vec3_make(pos.x << 16, pos.y << 16, pos.z << 16);
    cube->size = x_vec3_make(width, height, depth);
    
    cube->angularVelocity = x_vec3_make(0, 0, 0);
    cube->linearVelocity = x_vec3_make(0, 0, 0);
    cube->orientation = x_quaternion_identity();
    
    cube->mass = mass;
    cube->invMass = x_fp16x16_div(X_FP16x16_ONE, mass);
    
    calculate_inverse_static_inertia(cube);
    
    reset_forces(cube);
    
    update_geometry(cube);
    
    return cube;
}

void x_cubeobject_update_position(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    cube->center = x_vec3_add_scaled(&cube->center, &cube->linearVelocity, deltaTime);
    
    update_orientation(cube, deltaTime);
}

static void update_velocity(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    cube->linearVelocity = x_vec3_add_scaled(&cube->linearVelocity, &cube->force, x_fp16x16_mul(deltaTime, cube->invMass));
    
    //x_vec3_print(&cube->linearVelocity, "Linear velocity");
    
    X_Vec3_fp16x16 dAngularVelocity;
    x_mat4x4_transform_vec3_fp16x16(&cube->inverseInertia, &cube->torque, &dAngularVelocity);
    
    cube->angularVelocity = x_vec3_add_scaled(&cube->angularVelocity, &dAngularVelocity, deltaTime);
}

static void calculate_velocity_of_point(const X_CubeObject* cube, const X_Vec3* point, X_Vec3_fp16x16* velocityDest)
{
    X_Vec3 center = x_vec3_fp16x16_to_vec3(&cube->center);
    X_Vec3 relativeToCenter = x_vec3_sub(point, &center);
    X_Vec3 rotationalVelocity = x_vec3_cross(&cube->angularVelocity, &relativeToCenter);
    
    *velocityDest = x_vec3_add(&cube->linearVelocity, &rotationalVelocity);
}

static void handle_collision_with_xz_plane(X_CubeObject* cube)
{
    int maxPen = 0;
    
    int hitV[8];
    int totalHitV = 0;
    
    for(int i = 0; i < 8; ++i)
    {
        if(cube->geometry.vertices[i].y > 0)
        {
            hitV[totalHitV++] = i;
            maxPen = X_MAX(maxPen, cube->geometry.vertices[i].y);
        }
    }

    x_fp16x16 force;
    
    _Bool collide = totalHitV > 0;
    if(collide)
    {
        for(int i = 0; i < totalHitV; ++i)
        {
            X_Vec3_fp16x16 force = x_vec3_make(0, -2 * cube->linearVelocity.y / totalHitV, 0);
            x_cubeobject_apply_force(cube, force, cube->geometry.vertices[hitV[i]]);
        }
        
        //cube->linearVelocity.y = -cube->linearVelocity.y;
        cube->center.y -= (maxPen) << 16;
    }
}

void x_cubeobject_update(X_CubeObject* cube, x_fp16x16 deltaTime)
{
    cube->linearVelocity.y += 65536 * 1;
    
    handle_collision_with_xz_plane(cube);
    calculate_inverse_inertia(cube);
    update_velocity(cube, deltaTime);
    reset_forces(cube);
    x_cubeobject_update_position(cube, deltaTime);
    update_geometry(cube);
}

void x_cubeobject_render(X_CubeObject* cube, X_RenderContext* rcontext, X_Color color)
{
    x_cube_render(&cube->geometry, rcontext, color);
}

void x_cubeobject_apply_force(X_CubeObject* cube, X_Vec3_fp16x16 force, X_Vec3 pointOfContact)
{
    cube->force = x_vec3_add(&cube->force, &force);
    
    X_Vec3 center = x_vec3_fp16x16_to_vec3(&cube->center);
    X_Vec3 pointRelative = x_vec3_sub(&pointOfContact, &center);
    X_Vec3_fp16x16 torque = x_vec3_cross(&pointRelative, &force);
    
    cube->torque = x_vec3_add(&cube->torque, &torque);
    
    x_vec3_print(&torque, "Torque");
    x_vec3_print(&pointRelative, "Relative");
}
