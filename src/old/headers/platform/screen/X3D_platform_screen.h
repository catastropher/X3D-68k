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
//
//
//
// X3D_platform_screen.c -> interface for screen functionality (flipping, drawing, etc)

#pragma once

#include "common/X3D_interface.h"
#include "common/X3D_int.h"

typedef uint16 X3D_Color;

X3D_IMPLEMENTATION void x3dplatform_screen_flip();
X3D_IMPLEMENTATION void x3dplatform_screen_clear(X3D_Color color);
X3D_IMPLEMENTATION void x3dplatform_draw_pix(int16 x, int16 y, X3D_Color color);
X3D_IMPLEMENTATION void x3dplatform_draw_line(int16 x0, int16 y0, int16 x1, int16 y1, X3D_Color color);
X3D_IMPLEMENTATION X3D_Color x3dplatform_rgb_to_color(uint8 r, uint8 g, uint8 b);
X3D_IMPLEMENTATION void x3dplatform_color_to_rgb(X3D_Color color, uint8* r, uint8* g, uint8* b);

