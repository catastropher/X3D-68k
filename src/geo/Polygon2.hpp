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

#include "Vec2.hpp"

struct Polygon2
{
    Polygon2(X_Vec2* vertices_, int totalVertices_) :
        vertices(vertices_),
        totalVertices(totalVertices_) { }
    
    X_Vec2* vertices;
    int totalVertices;
};

struct LevelPolygon2 : Polygon2
{
    LevelPolygon2(X_Vec2* vertices_, int totalVertices_, int* edgeIds_)
    : Polygon2(vertices_, totalVertices_),
    edgeIds(edgeIds_) { }
    
    int* edgeIds;
};

