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

#include "geo/Vec3.hpp"
#include "geo/Vec4.hpp"
#include "math/Angle.hpp"
#include "math/Mat4x4.hpp"

typedef X_Vec4_fp16x16 X_Quaternion;

template<typename T>
struct QuaternionTemplate : public Vec4Template<T>
{
    constexpr QuaternionTemplate(T x, T y, T z, T w)
        : Vec4Template<T>(x, y, z, w)
    {

    }

    QuaternionTemplate()
    {
        *this = identity();
    }

    void toMat4x4(Mat4x4& dest) const;

    QuaternionTemplate<T> operator*(QuaternionTemplate<T>& q) const;

    static constexpr QuaternionTemplate identity()
    {
        return QuaternionTemplate(0, 0, 0, convert<T>(1.0f));
    }

    static QuaternionTemplate fromAxisAngle(const Vec3Template<T>& axis, fp angle);
    static QuaternionTemplate fromEulerAngles(fp x, fp y, fp z);
};

using Quaternion = QuaternionTemplate<fp>;

