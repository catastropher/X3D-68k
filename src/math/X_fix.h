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

typedef int x_fp16x16;
typedef int x_fp24x8;
typedef long long int x_fp32x32;
typedef int x_fp2x30;

#define X_FP16x16_ONE 0x10000
#define X_FP16x16_HALF (X_FP16x16_ONE / 2)

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

////////////////////////////////////////////////////////////////////////////////
/// Divides two @ref x_fp16x16 numbers.
///
/// @param n    - numerator
/// @param d    - denominator
///
/// @return n / d as an x_fp16x16.
///
/// @note Be wary of overflow.
/// @note This will cause division by zero if d == 0.
/// @note This is a fairly expensive operation as it involves 64-bit division.
////////////////////////////////////////////////////////////////////////////////
static inline x_fp16x16 x_fp16x16_div(x_fp16x16 n, x_fp16x16 d)
{
    return ((x_fp32x32)n << 16) / d;
}

static inline x_fp16x16 x_fp16x16_mul(x_fp16x16 a, x_fp16x16 b)
{
    return ((x_fp32x32)a * b) >> 16;
}

static inline x_fp16x16 x_fp16x16_mul_three(x_fp16x16 a, x_fp16x16 b, x_fp16x16 c)
{
    return x_fp16x16_mul(a, x_fp16x16_mul(b, c));
}

static inline int x_int_lerp(int x0, int x1, x_fp16x16 t)
{
    return x0 + (((x1 - x0) * t) >> 16);
}

static inline x_fp16x16 x_fp16x16_lerp(x_fp16x16 x0, x_fp16x16 x1, x_fp16x16 t)
{
    return x0 + x_fp16x16_mul(x1 - x0, t);
}

static inline float x_fp16x16_to_float(x_fp16x16 val)
{
    return val / 65536.0;
}

static inline x_fp16x16 x_fp16x16_from_float(float val)
{
    return val * 65536;
}

static inline x_fp16x16 x_fp16x16_clamp(x_fp16x16 val, x_fp16x16 minValue, x_fp16x16 maxValue)
{
    if(val < minValue)
        val = minValue;
    else if(val > maxValue)
        val = maxValue;
    
    return val;
}

static inline x_fp16x16 x_int_div_as_fp16x16(int a, int b)
{
    return (a << 16) / b;
}

// Calculates the reciprocal of a number in the range 1...65536
static inline x_fp16x16 x_fastrecip(unsigned int val)
{
    int shiftUp = __builtin_clz(val) - 16;
    val <<= shiftUp;
    
    const unsigned int ADD = (48 << 16) / 17;
    
    unsigned int x = ADD - (val << 1);
    
    x = x + x_fp16x16_mul(x, X_FP16x16_ONE - x_fp16x16_mul(val, x));
    x = x + x_fp16x16_mul(x, X_FP16x16_ONE - x_fp16x16_mul(val, x));
    x = x + x_fp16x16_mul(x, X_FP16x16_ONE - x_fp16x16_mul(val, x));
    
    return x >> (16 - shiftUp);
}

