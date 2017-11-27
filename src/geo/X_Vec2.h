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

////////////////////////////////////////////////////////////////////////////////
/// A 2D vector or vertex.
////////////////////////////////////////////////////////////////////////////////
typedef struct X_Vec2
{
    int x;
    int y;
} X_Vec2;

typedef X_Vec2 X_Vec2_fp16x16;

////////////////////////////////////////////////////////////////////////////////
/// Determines whether two 2D vectors are equal.
////////////////////////////////////////////////////////////////////////////////
static inline _Bool x_vec2_equal(const X_Vec2* a, const X_Vec2* b)
{
    return a->x == b->x && a->y == b->y;
}

static inline X_Vec2 x_vec2_make(int x, int y)
{
    return (X_Vec2) { x, y };
}

