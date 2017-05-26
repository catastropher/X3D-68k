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

