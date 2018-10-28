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

#include <cstdint>

#include "Sqrt.hpp"

namespace X3D
{
    struct fp
    {
        fp() { }

        constexpr fp(int val_) : val(val_) { }
        
        friend fp operator+(fp a, fp b);
        friend fp operator+(fp a, int b);
        friend fp operator+(int a, fp b);
        
        friend fp operator-(fp a, fp b);
        friend fp operator-(fp a, int b);
        friend fp operator-(int a, fp b);
        friend fp operator-(fp f);
        
        friend fp operator*(int a, const fp b);
        friend fp operator*(const fp a, int b);
        friend fp operator*(const fp a, const fp b);
        
        friend fp operator/(const fp a, const fp b);
        friend fp operator/(const fp a, int b);
        
        friend bool operator<(fp a, fp b);
        friend bool operator<=(fp a, fp b);
        friend bool operator>(fp a, fp b);
        friend bool operator>=(fp a, fp b);
        friend bool operator==(fp a, fp b);
        friend bool operator!=(fp a, fp b);

        fp whole()
        {
            return val & 0x7FFF0000;
        }
        
        fp frac()
        {
            return val & 0x0000FFFF;
        }
        
        int toInt() const
        {
            return val >> 16;
        }
        
        float toFloat() const
        {
            return val / 65536.0;
        }
        
        int internalValue() const
        {
            return val;
        }
        
        static fp lerp(fp x0, fp x1, fp t)
        {
            return x0 + (x1 - x0) * t;
        }
        
        static fp fromInt(int value)
        {
            return fp(value << 16);
        }
        
        static fp fromFloat(float value)
        {
            return fp(value * 65536.0);
        }

        fp sqrt() const
        {
            return fp(fastSqrt(val) << 8);
        }
        
        int val;

        static const fp ONE;
    };

    const fp fp::ONE = 1 << 16;

    // Addition
    inline fp operator+(fp a, fp b)
    {
        return fp(a.val + b.val);
    }

    inline fp operator+(fp a, int b)
    {
        return a + fp::fromInt(b);
    }

    inline fp operator+(int a, fp b)
    {
        return fp::fromInt(a) + b;
    }

    // Subtraction
    inline fp operator-(fp a, fp b)
    {
        return fp(a.val - b.val);
    }

    inline fp operator-(fp a, int b)
    {
        return a - fp::fromInt(b);
    }

    inline fp operator-(int a, fp b)
    {
        return fp::fromInt(a) - b;
    }

    inline fp& operator+=(fp& a, fp b)
    {
        a.val += b.val;

        return a;
    }

    inline fp& operator-=(fp& a, fp b)
    {
        a.val -= b.val;

        return a;
    }

    inline fp operator-(fp f)
    {
        return fp(-f.val);
    }

    // Multiplication
    inline fp operator*(int a, const fp b)
    {
        return fp(a * b.val);
    }

    inline fp operator*(const fp a, int b)
    {
        return fp(a.val * b);
    }

    inline fp operator*(const fp a, const fp b)
    {
        return fp(((int64_t)a.val * b.val) >> 16);
    }

    // Division
    inline fp operator/(const fp a, const fp b)
    {
        return fp(((int64_t)a.val << 16) / b.val);
    }

    inline fp operator/(const fp a, int b)
    {
        return fp(a.val / b);
    }

    // Comparision
    inline bool operator<(fp a, fp b)
    {
        return a.val < b.val;
    }

    inline bool operator<=(fp a, fp b)
    {
        return a.val <= b.val;
    }

    inline bool operator>(fp a, fp b)
    {
        return a.val > b.val;
    }

    inline bool operator>=(fp a, fp b)
    {
        return a.val >= b.val;
    }

    inline bool operator==(fp a, fp b)
    {
        return a.val == b.val;
    }

    inline bool operator!=(fp a, fp b)
    {
        return a.val != b.val;
    }
}

