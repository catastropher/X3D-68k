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

#include "math/Convert.hpp"
#include "math/FastSqrt.hpp"

typedef int x_fp16x16;
typedef int x_fp24x8;
typedef long long int x_fp32x32;
typedef int x_fp2x30;
typedef short x_fp0x16;

#define X_FP16x16_ONE 0x10000
#define X_FP16x16_HALF (X_FP16x16_ONE / 2)

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

    constexpr x_fp16x16 toFp16x16() const
    {
        return val;
    }

    constexpr int internalValue() const
    {
        return val;
    }
    
    static constexpr fp lerp(fp x0, fp x1, fp t)
    {
        return x0 + (x1 - x0) * t;
    }
    
    static constexpr fp fromInt(int value)
    {
        return fp(value << 16);
    }
    
    static constexpr fp fromFloat(float value)
    {
        return fp(value * 65536.0);
    }

    fp sqrt() const
    {
        return fp(x_sqrt(val) << 8);
    }

    constexpr fp ceil() const
    {
        return fp((val & 0xFFFF0000) + ((val & 0x0000FFFF) != 0 ? fp::fromInt(1).val : 0));
    }

    constexpr int asRightShiftedInteger(int rightShift) const
    {
       return val >> rightShift;
    }
    
    int val;
};

constexpr fp operator "" _fp(long double val)
{
    return fp::fromFloat((float)val);
}


// Addition
inline constexpr fp operator+(fp a, fp b)
{
    return fp(a.val + b.val);
}

inline constexpr fp operator+(fp a, int b)
{
    return a + fp::fromInt(b);
}

inline constexpr fp operator+(int a, fp b)
{
    return fp::fromInt(a) + b;
}

// Subtraction
inline constexpr fp operator-(fp a, fp b)
{
    return fp(a.val - b.val);
}

inline constexpr fp operator-(fp a, int b)
{
    return a - fp::fromInt(b);
}

inline constexpr fp operator-(int a, fp b)
{
    return fp::fromInt(a) - b;
}

inline constexpr fp& operator+=(fp& a, fp b)
{
    a.val += b.val;

    return a;
}

inline constexpr fp& operator-=(fp& a, fp b)
{
    a.val -= b.val;

    return a;
}

inline constexpr fp operator<<(fp f, int amount)
{
    return fp(f.internalValue() << amount);
}

inline constexpr fp operator>>(fp f, int amount)
{
    return fp(f.internalValue() >> amount);
}

inline constexpr fp operator-(fp f)
{
    return fp(-f.val);
}

// Multiplication
inline constexpr fp operator*(int a, const fp b)
{
    return fp(a * b.val);
}

inline constexpr fp operator*(const fp a, int b)
{
    return fp(a.val * b);
}

inline constexpr fp operator*(const fp a, const fp b)
{
    return fp(((int64_t)a.val * b.val) >> 16);
}

// Division
inline constexpr fp operator/(const fp a, const fp b)
{
    return fp(((int64_t)a.val << 16) / b.val);
}

inline constexpr fp operator/(const fp a, int b)
{
    return fp(a.val / b);
}

// Comparision
inline constexpr bool operator<(fp a, fp b)
{
    return a.val < b.val;
}

inline constexpr bool operator<=(fp a, fp b)
{
    return a.val <= b.val;
}

inline constexpr bool operator>(fp a, fp b)
{
    return a.val > b.val;
}

inline constexpr bool operator>=(fp a, fp b)
{
    return a.val >= b.val;
}

inline constexpr bool operator==(fp a, fp b)
{
    return a.val == b.val;
}

inline constexpr bool operator!=(fp a, fp b)
{
    return a.val != b.val;
}

////////////////////////////////////////////////////////////////////////////////
/// Creates an @ref x_fp16x16 from a whole number.
////////////////////////////////////////////////////////////////////////////////
static inline x_fp16x16 x_fp16x16_from_int(int val)
{
    return val << 16;
}

static inline int x_fp16x16_to_int(x_fp16x16 val)
{
    return val >> 16;
}

static inline x_fp16x16 x_fp16x16_mul(x_fp16x16 a, x_fp16x16 b)
{
    return ((x_fp32x32)a * b) >> 16;
}

static inline float x_fp16x16_to_float(x_fp16x16 val)
{
    return val / 65536.0;
}

static inline x_fp16x16 x_fp16x16_from_float(float val)
{
    return val * 65536;
}

// Calculates the reciprocal of a number in the range 1...65536
static inline int x_fastrecip(unsigned int val)
{
    int shiftUp = __builtin_clz(val) - 16;
    fp newVal = (val << shiftUp);

    const fp ADD = 48.0_fp / 17;

    fp x = ADD - (newVal << 1);

    x = x + x * (1.0_fp - newVal * x);
    x = x + x * (1.0_fp - newVal * x);
    x = x + x * (1.0_fp - newVal * x);

    return x.asRightShiftedInteger(16 - shiftUp);
}

static inline int mul(int a, int b)
{
    return ((long long)a * b) >> 16;
}

#include <cstdio>

static inline x_fp16x16 x_fastrecip_unshift(unsigned int val, int& shift)
{
    const int bits = 16;
    int shiftDown =  (32 - bits) - __builtin_clz(val);
    val >>= shiftDown;

    const unsigned int ADD = (48 << bits) / 17;

    unsigned int x = ADD - (val << 1);

    const int ONE = 1 << bits;

    x = x + mul(x, ONE - mul(val, x));
    x = x + mul(x, ONE - mul(val, x));
    x = x + mul(x, ONE - mul(val, x));

    shift = (shiftDown + bits - (32 - bits));

    return x;
}


template<>
inline void convert(fp& from, float& to)
{
    to = from.toFloat();
}

template<>
inline void convert(float& from, fp& to)
{
    to = fp::fromFloat(from);
}

template<>
inline void convert(fp& from, x_fp16x16& to)
{
    to = from.internalValue();
}

template<>
inline void convert(int& from, fp& to)
{
    to = fp::fromInt(from);
}

inline fp abs(fp val)
{
    return val < 0 ? -val : val;
}

