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

#include "Quaternion.hpp"
#include "math/Trig.hpp"

template struct QuaternionTemplate<fp>;
template struct QuaternionTemplate<float>;

template<typename T>
QuaternionTemplate<T> QuaternionTemplate<T>::fromAxisAngle(const Vec3Template<T>& axis, fp angle)
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

template<typename T>
void QuaternionTemplate<T>::toMat4x4(Mat4x4& dest) const
{
    auto xx = this->x * this->x;
    auto xy = this->x * this->y;
    auto xz = this->x * this->z;
    auto xw = this->x * this->w;
    
    auto yy = this->y * this->y;
    auto yz = this->y * this->z;
    auto yw = this->y * this->w;
    
    auto zz = this->z * this->z;
    auto zw = this->z * this->w;

    const fp ONE = fp::fromInt(1);
    
    dest.elem[0][0] = ONE - convert<fp>(2 * (yy + zz));
    dest.elem[0][1] =       convert<fp>(2 * (xy - zw));
    dest.elem[0][2] =       convert<fp>(2 * (xz + yw));
    dest.elem[0][3] = 0;
    
    dest.elem[1][0] =       convert<fp>(2 * (xy + zw));
    dest.elem[1][1] = ONE - convert<fp>(2 * (xx + zz));
    dest.elem[1][2] =       convert<fp>(2 * (yz - xw));
    dest.elem[1][3] = 0;
    
    dest.elem[2][0] =       convert<fp>(2 * (xz - yw));
    dest.elem[2][1] =       convert<fp>(2 * (yz + xw));
    dest.elem[2][2] = ONE - convert<fp>(2 * (xx + yy));
    dest.elem[2][3] = 0;
    
    dest.elem[3][0] = 0;
    dest.elem[3][1] = 0;
    dest.elem[3][2] = 0;
    dest.elem[3][3] = ONE;
}

template<typename T>
QuaternionTemplate<T> QuaternionTemplate<T>::fromEulerAngles(fp x, fp y, fp z)
{
    T one = convert<T>(1.0f);

    Vec3Template<T> xAxis(one, 0, 0);
    QuaternionTemplate<T> xRotation = QuaternionTemplate<T>::fromAxisAngle(xAxis, x);

    Vec3Template<T> yAxis(0, one, 0);
    QuaternionTemplate<T> yRotation = QuaternionTemplate<T>::fromAxisAngle(yAxis, y);

    return xRotation * yRotation;
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

