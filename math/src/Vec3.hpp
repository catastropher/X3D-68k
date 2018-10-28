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

#include "Fp.hpp"
#include "Convert.hpp"

namespace X3D
{
    template<typename T>
    struct Vec3Template
    {
        Vec3Template(T x_, T y_, T z_)
            : x(x_),
            y(y_),
            z(z_)
        {

        }

        Vec3Template()
            : x(0),
            y(0),
            z(0)
        {

        }
        
        Vec3Template operator+(const Vec3Template& v) const
        {
            return Vec3Template(x + v.x, y + v.y, z + v.z);
        }
        
        Vec3Template operator-(const Vec3Template& v) const
        {
            return Vec3Template(x - v.x, y - v.y, z - v.z);
        }
        
        Vec3Template operator+=(const Vec3Template& v)
        {
            *this = *this + v;
            return *this;
        }

        Vec3Template toX3dCoords() const
        {
            return Vec3Template(y, -z, -x);
        }

        T dot(const Vec3Template& v) const
        {
            return x * v.x + y * v.y + z * v.z;
        }

        Vec3Template operator-() const
        {
            return Vec3Template(-x, -y, -z);
        }
        
        Vec3Template cross(const Vec3Template& v) const
        {
            return Vec3Template(
                y * v.z - v.y * z,
                z * v.x - v.z * x,
                x * v.y - v.x * y);
        }

        bool operator==(const Vec3Template& v) const
        {
            return x == v.x && y == v.y && z == v.z;
        }

        bool operator!=(const Vec3Template& v) const
        {
            return !(*this == v);
        }

        void print(const char* name) const
        {
            printf("%s: %f %f %f\n", name, convert<float>(x), convert<float>(y), convert<float>(z));
        }

        void normalize();

        // FIXME: don't use floats
        T length()
        {
            float xx = convert<float>(x);
            float yy = convert<float>(y);
            float zz = convert<float>(z);

            return convert<T>(sqrtf(xx * xx + yy * yy + zz * zz));
        }

        static Vec3Template origin()
        {
            return Vec3Template(0, 0, 0);
        }
        
        T x;
        T y;
        T z;
    };

    // Template specializations
    using Vec3 = Vec3Template<fp>;
    using Vec3f = Vec3Template<float>;
}

