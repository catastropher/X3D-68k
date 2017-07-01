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

#include <math.h>

#include "X_Quaternion.h"
#include "math/X_trig.h"

void x_quaternion_init_from_axis_angle(X_Quaternion* quat, const X_Vec3_fp16x16* axis, x_angle256 angle)
{
    x_fp16x16 cosAngle = x_cos(angle / 2);
    x_fp16x16 sinAngle = x_sin(angle / 2);
    
    quat->x = x_fp16x16_mul(axis->x, sinAngle);
    quat->y = x_fp16x16_mul(axis->y, sinAngle);
    quat->z = x_fp16x16_mul(axis->z, sinAngle);
    quat->w = cosAngle;
}

void x_quaternion_mul(const X_Quaternion* a, const X_Quaternion* b, X_Quaternion* dest)
{
    dest->x =  x_fp16x16_mul(a->x, b->w) + x_fp16x16_mul(a->y, b->z) - x_fp16x16_mul(a->z, b->y) + x_fp16x16_mul(a->w, b->x);
    dest->y = -x_fp16x16_mul(a->x, b->z) + x_fp16x16_mul(a->y, b->w) + x_fp16x16_mul(a->z, b->x) + x_fp16x16_mul(a->w, b->y);
    dest->z =  x_fp16x16_mul(a->x, b->y) - x_fp16x16_mul(a->y, b->x) + x_fp16x16_mul(a->z, b->w) + x_fp16x16_mul(a->w, b->z);
    dest->w = -x_fp16x16_mul(a->x, b->x) - x_fp16x16_mul(a->y, b->y) - x_fp16x16_mul(a->z, b->z) + x_fp16x16_mul(a->w, b->w);
}

void x_quaternion_to_mat4x4(const X_Quaternion* src, X_Mat4x4* dest)
{
    x_fp16x16 xx = x_fp16x16_mul(src->x, src->x);
    x_fp16x16 xy = x_fp16x16_mul(src->x, src->y);
    x_fp16x16 xz = x_fp16x16_mul(src->x, src->z);
    x_fp16x16 xw = x_fp16x16_mul(src->x, src->w);
    
    x_fp16x16 yy = x_fp16x16_mul(src->y, src->y);
    x_fp16x16 yz = x_fp16x16_mul(src->y, src->z);
    x_fp16x16 yw = x_fp16x16_mul(src->y, src->w);
    
    x_fp16x16 zz = x_fp16x16_mul(src->z, src->z);
    x_fp16x16 zw = x_fp16x16_mul(src->z, src->w);
    
    dest->elem[0][0] = X_FP16x16_ONE - 2 * (yy + zz);
    dest->elem[0][1] =                 2 * (xy - zw);
    dest->elem[0][2] =                 2 * (xz + yw);
    dest->elem[0][3] = 0;
    
    dest->elem[1][0] =                 2 * (xy + zw);
    dest->elem[1][1] = X_FP16x16_ONE - 2 * (xx + zz);
    dest->elem[1][2] =                 2 * (yz - xw);
    dest->elem[1][3] = 0;
    
    dest->elem[2][0] =                 2 * (xz - yw);
    dest->elem[2][1] =                 2 * (yz + xw);
    dest->elem[2][2] = X_FP16x16_ONE - 2 * (xx + yy);
    dest->elem[2][3] = 0;
    
    dest->elem[3][0] = 0;
    dest->elem[3][1] = 0;
    dest->elem[3][2] = 0;
    dest->elem[3][3] = X_FP16x16_ONE;
}

void x_quaternion_init_from_euler_angles(X_Quaternion* quat, x_angle256 x, x_angle256 y, x_angle256 z)
{
    y = X_ANG_180 - y;
    z = X_ANG_180 - z;
    
    x_fp16x16 t0 = x_cos(x / 2);
    x_fp16x16 t1 = x_sin(x / 2);
    x_fp16x16 t2 = x_cos(z / 2);
    x_fp16x16 t3 = x_sin(z / 2);
    x_fp16x16 t4 = x_cos(y / 2);
    x_fp16x16 t5 = x_sin(y / 2);
    
    quat->x = x_fp16x16_mul_three(t0, t2, t4) + x_fp16x16_mul_three(t1, t3, t5);
    quat->y = x_fp16x16_mul_three(t0, t3, t4) - x_fp16x16_mul_three(t1, t2, t5);
    quat->z = x_fp16x16_mul_three(t0, t2, t5) + x_fp16x16_mul_three(t1, t3, t4);
    quat->w = x_fp16x16_mul_three(t1, t2, t4) - x_fp16x16_mul_three(t0, t3, t5);
}

void x_quaternion_normalize(X_Quaternion* quat)
{
    /// @todo rewrite without floats
    float len = sqrt((float)quat->x * quat->x + (float)quat->y * quat->y + (float)quat->z * quat->z + (float)quat->w * quat->w);
    
    quat->x = (quat->x / len) * 65536.0;
    quat->y = (quat->y / len) * 65536.0;
    quat->z = (quat->z / len) * 65536.0;
    quat->w = (quat->w / len) * 65536.0;
}

