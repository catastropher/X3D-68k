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
#include "Limits.hpp"

namespace X3D
{
    struct fp
    {
        fp() { }

        constexpr fp(int val_) : val(val_) { }
        
        friend constexpr fp operator+(fp a, fp b);
        friend constexpr fp operator+(fp a, int b);
        friend constexpr fp operator+(int a, fp b);
        
        friend constexpr fp operator-(fp a, fp b);
        friend constexpr fp operator-(fp a, int b);
        friend constexpr fp operator-(int a, fp b);
        friend constexpr fp operator-(fp f);
        
        friend constexpr fp operator*(int a, const fp b);
        friend constexpr fp operator*(const fp a, int b);
        friend constexpr fp operator*(const fp a, const fp b);
        
        friend constexpr fp operator/(const fp a, const fp b);
        friend constexpr fp operator/(const fp a, int b);
        
        friend constexpr bool operator<(fp a, fp b);
        friend constexpr bool operator<=(fp a, fp b);
        friend constexpr bool operator>(fp a, fp b);
        friend constexpr bool operator>=(fp a, fp b);
        friend constexpr bool operator==(fp a, fp b);
        friend constexpr bool operator!=(fp a, fp b);

        constexpr fp whole() const
        {
            return val & 0x7FFF0000;
        }
        
        constexpr fp frac() const
        {
            return val & 0x0000FFFF;
        }
        
        constexpr int toInt() const
        {
            return val >> 16;
        }
        
        constexpr float toFloat() const
        {
            return val / 65536.0;
        }
        
        constexpr int internalValue() const
        {
            return val;
        }
        
        constexpr static fp lerp(fp x0, fp x1, fp t)
        {
            return x0 + (x1 - x0) * t;
        }
        
        constexpr static fp fromInt(int value)
        {
            return fp(value << 16);
        }
        
        constexpr static fp fromFloat(float value)
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
    constexpr inline fp operator+(fp a, fp b)
    {
        return fp(a.val + b.val);
    }

    constexpr inline fp operator+(fp a, int b)
    {
        return a + fp::fromInt(b);
    }

    constexpr inline fp operator+(int a, fp b)
    {
        return fp::fromInt(a) + b;
    }

    // Subtraction
    constexpr inline fp operator-(fp a, fp b)
    {
        return fp(a.val - b.val);
    }

    constexpr inline fp operator-(fp a, int b)
    {
        return a - fp::fromInt(b);
    }

    constexpr inline fp operator-(int a, fp b)
    {
        return fp::fromInt(a) - b;
    }

    constexpr inline fp& operator+=(fp& a, fp b)
    {
        a.val += b.val;

        return a;
    }

    constexpr inline fp& operator-=(fp& a, fp b)
    {
        a.val -= b.val;

        return a;
    }

    constexpr inline fp operator-(fp f)
    {
        return fp(-f.val);
    }

    // Multiplication
    constexpr inline fp operator*(int a, const fp b)
    {
        return fp(a * b.val);
    }

    constexpr inline fp operator*(const fp a, int b)
    {
        return fp(a.val * b);
    }

    constexpr inline fp operator*(const fp a, const fp b)
    {
        return fp(((int64_t)a.val * b.val) >> 16);
    }

    // Division
    constexpr inline fp operator/(const fp a, const fp b)
    {
        return fp(((int64_t)a.val << 16) / b.val);
    }

    constexpr inline fp operator/(const fp a, int b)
    {
        return fp(a.val / b);
    }

    // Comparision
    constexpr inline bool operator<(fp a, fp b)
    {
        return a.val < b.val;
    }

    constexpr inline bool operator<=(fp a, fp b)
    {
        return a.val <= b.val;
    }

    constexpr inline bool operator>(fp a, fp b)
    {
        return a.val > b.val;
    }

    constexpr inline bool operator>=(fp a, fp b)
    {
        return a.val >= b.val;
    }

    constexpr inline bool operator==(fp a, fp b)
    {
        return a.val == b.val;
    }

    constexpr inline bool operator!=(fp a, fp b)
    {
        return a.val != b.val;
    }

    template<>
    inline constexpr fp minValue()
    {
        return fp(minValue<int>());
    }

    template<>
    inline constexpr fp maxValue()
    {
        return fp(maxValue<int>());
    }
}

