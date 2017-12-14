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

#include <stdio.h>
#include <stdlib.h>

#include "X_Vec3.h"
#include "util/X_util.h"
#include "math/X_fix.h"

static inline void scale_components_to_less_than_one_half(X_Vec3_fp16x16* v)
{
    x_fp16x16 maxValue = X_MAX(abs(v->x), X_MAX(abs(v->y), abs(v->z)));
    
    while(maxValue >= X_FP16x16_HALF)
    {
        v->x >>= 1;
        v->y >>= 1;
        v->z >>= 1;
        maxValue >>= 1;
    }
}

void x_vec3_fp16x16_normalize(X_Vec3_fp16x16* v)
{
    scale_components_to_less_than_one_half(v);
    int len = x_sqrt(v->x * v->x + v->y * v->z + v->z * v->z);
    
    v->x = (v->x << 16) / len;
    v->y = (v->y << 16) / len;
    v->z = (v->z << 16) / len;
}

void x_vec3_print(const X_Vec3* v, const char* label)
{
    printf("%s: { %d, %d, %d }\n", label, v->x, v->y, v->z);
}

void x_vec3_fp16x16_print(const X_Vec3_fp16x16* v, const char* label)
{
    printf("%s: { %f, %f, %f }\n", label, x_fp16x16_to_float(v->x), x_fp16x16_to_float(v->y), x_fp16x16_to_float(v->z));
}

