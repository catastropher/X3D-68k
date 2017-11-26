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

#include "geo/X_Vec3.h"
#include "geo/X_Vec4.h"
#include "math/X_angle.h"
#include "math/X_Mat4x4.h"

typedef X_Vec4_fp16x16 X_Quaternion;

void x_quaternion_init_from_axis_angle(X_Quaternion* quat, const X_Vec3_fp16x16* axis, x_fp16x16 angle);
void x_quaternion_mul(const X_Quaternion* a, const X_Quaternion* b, X_Quaternion* dest);
void x_quaternion_to_mat4x4(const X_Quaternion* src, X_Mat4x4* dest);
void x_quaternion_init_from_euler_angles(X_Quaternion* quat, x_fp16x16 x, x_fp16x16 y, x_fp16x16 z);
void x_quaternion_normalize(X_Quaternion* quat);

static inline X_Quaternion x_quaternion_make(x_fp16x16 x, x_fp16x16 y, x_fp16x16 z, x_fp16x16 w)
{
    return x_vec4_make(x, y, z, w);
}

static inline X_Quaternion x_quaternion_identity(void)
{
    return x_quaternion_make(0, 0, 0, X_FP16x16_ONE);
}

static inline X_Quaternion x_quaternion_conjugate(X_Quaternion* quat)
{
    return x_quaternion_make(-quat->x, -quat->y, -quat->z, quat->w);
}

static inline x_fp16x16 x_quaternion_dot(const X_Quaternion* a, const X_Quaternion* b)
{
    return x_fp16x16_mul(a->x, b->x) + 
        x_fp16x16_mul(a->y, b->y) + 
        x_fp16x16_mul(a->z, b->z) + 
        x_fp16x16_mul(a->w, b->w);
}

static inline X_Quaternion x_quaternion_scale(const X_Quaternion* quat, x_fp16x16 scale)
{
    return x_quaternion_make
    (
        x_fp16x16_mul(quat->x, scale),
        x_fp16x16_mul(quat->y, scale),
        x_fp16x16_mul(quat->z, scale),
        x_fp16x16_mul(quat->w, scale)
    );
}

static inline X_Quaternion x_quaternion_add(const X_Quaternion* a, const X_Quaternion* b)
{
    return x_vec4_add(a, b);
}

