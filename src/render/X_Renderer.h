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

#include "X_Texture.h"
#include "dev/X_Console.h"
#include "X_activeedge.h"

#define X_RENDERER_FILL_DISABLED -1

typedef struct X_Renderer
{
    X_AE_Context activeEdgeContext;
    
    int fillColor;
    _Bool showFps;
    
    X_ConsoleVar varFillColor;
    X_ConsoleVar varShowFps;
} X_Renderer;

void x_renderer_init(X_Renderer* renderer, X_Console* console);
