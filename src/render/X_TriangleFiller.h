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

#include "geo/X_Vec2.h"
#include "X_Texture.h"
#include "math/X_fix.h"
#include "X_RenderContext.h"

typedef enum X_TriangleFillerType
{
    X_TRIANGLE_FLAT_TOP = 0,
    X_TRIANGLE_FLAT_BOTTOM = 1,
    X_TRIANGLE_GENERIC = 2
} X_TriangleFillerType;

typedef struct X_TriangleFillerVertex
{
    X_Vec2 v;
} X_TriangleFillerVertex;

typedef struct X_TriangleFillerEdge
{
    x_fp16x16 x;
    x_fp16x16 xSlope;
    int endY;
} X_TriangleFillerEdge;

typedef struct X_TriangleFiller
{
    X_TriangleFillerVertex vertices[3];
    X_TriangleFillerType type;
    
    X_Color fillColor;
    int y;
    int endY;
    
    X_TriangleFillerEdge edges[3];
    int nextEdge;
    
    X_TriangleFillerEdge* leftEdge;
    X_TriangleFillerEdge* rightEdge;
    X_TriangleFillerEdge** firstEndingEdge;
    
    X_TriangleFillerEdge* middleStartingEdge;
    
    X_RenderContext* renderContext;
} X_TriangleFiller;

