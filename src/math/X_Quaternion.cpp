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

template struct QuaternionTemplate<fp>;
template struct QuaternionTemplate<float>;

template<typename T>
QuaternionTemplate<T> QuaternionTemplate<T>::fromAxisAngle(Vec3Template<T>& axis, fp angle)
{
    QuaternionTemplate<T> q;

    fp cosAngle = x_cos(angle / 2);
    fp sinAngle = x_sin(angle / 2);

    q.x = convert<T>(axis.x * sinAngle);
    q.y = convert<T>(axis.y * sinAngle);
    q.z = convert<T>(axis.z * sinAngle);
    q.w = convert<T>(cosAngle);

    return q;
}

template<typename T>
QuaternionTemplate<T> QuaternionTemplate<T>::operator*(QuaternionTemplate<T>& q) const
{
    QuaternionTemplate<T> result;

    result.x = convert<T>(
        this->x * q.w
            + this->y * q.z
            - this->z * q.y
            + this->w * q.x);

    result.y = convert<T>(
        -this->x * q.z
            + this->y * q.w
            + this->z * q.x
            + this->w * q.y);

    result.z = convert<T>(
        this->x * q.y
            - this->y * q.x
            + this->z * q.w
            + this->w * q.z);
    
    result.w = convert<T>(
        -this->x * q.x
            - this->y * q.y
            - this->z * q.z
            + this->w * q.w);

    return result;
}

void x_quaternion_to_mat4x4(const X_Quaternion* src, Mat4x4* dest)
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

void x_quaternion_init_from_euler_angles(X_Quaternion* quat, fp x, fp y, fp z)
{
    y = X_ANG_180 - y;
    z = X_ANG_180 - z;
    
    x_fp16x16 t0 = x_cos(x / 2).toFp16x16();
    x_fp16x16 t1 = x_sin(x / 2).toFp16x16();
    x_fp16x16 t2 = x_cos(z / 2).toFp16x16();
    x_fp16x16 t3 = x_sin(z / 2).toFp16x16();
    x_fp16x16 t4 = x_cos(y / 2).toFp16x16();
    x_fp16x16 t5 = x_sin(y / 2).toFp16x16();
    
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

