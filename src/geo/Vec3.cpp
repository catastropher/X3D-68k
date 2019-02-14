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
#include <cmath>

#include "Vec3.hpp"
#include "util/Util.hpp"
#include "math/FixedPoint.hpp"

void x_vec3_normalize(Vec3* v)
{
    float x = fp(v->x).toFloat();
    float y = fp(v->y).toFloat();
    float z = fp(v->z).toFloat();

    float length = sqrtf(x * x + y * y + z * z);

    v->x = fp::fromFloat(x / length).toFp16x16();
    v->y = fp::fromFloat(y / length).toFp16x16();
    v->z = fp::fromFloat(z / length).toFp16x16();

    return;
}