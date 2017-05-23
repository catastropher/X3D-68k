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

#include "X_Vec3.h"
#include "util/X_util.h"
#include "math/X_fix.h"

static inline void scale_components_to_less_than_one_half(X_Vec3_fp16x16* v)
{
    x_fp16x16 maxValue = X_MAX(v->x, X_MAX(v->y, v->z));
    
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
    int len = x_vec3_length(v);
    
    v->x = (v->x << 16) / len;
    v->y = (v->y << 16) / len;
    v->z = (v->z << 16) / len;
}

