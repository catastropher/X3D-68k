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

#include "X3D_common.h"
#include "X3D_vector.h"

typedef struct X3D_ScreenManager {
  uint16 w;
  uint16 h;
  angle256 fov;
  int16 scale_x;
  int16 scale_y;
  
  X3D_Vex2D center;
  X3D_Vex2D pos;
} X3D_ScreenManager;

///////////////////////////////////////////////////////////////////////////////
/// Represents a color. This is platform-dependent and the meaning of the
///   contents may vary with what video mode is selected.
///////////////////////////////////////////////////////////////////////////////
typedef uint32 X3D_Color;


///////////////////////////////////////////////////////////////////////////////
/// Flips the screen's double buffer. Anything that was drawn to the buffer
///   will be visible after calling this.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM void x3d_screen_flip();


///////////////////////////////////////////////////////////////////////////////
/// Clears the screen.
///
/// @param color  - color to clear the screen to
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM void x3d_screen_clear(X3D_Color color);


///////////////////////////////////////////////////////////////////////////////
/// Draws a pixel on the screen.
///
/// @param x      - x coordinate
/// @param y      - y coordinate
/// @param color  - color of the pixel
///
/// @return Nothing.
/// @note Screen zoom factor affects the number of physical pixels that are
///   set. For example, if the screen zoom is 3, a 3x3 group of pixels will
///   be drawn.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM void x3d_screen_draw_pix(int16 x, int16 y, X3D_Color color);


///////////////////////////////////////////////////////////////////////////////
/// Draws a line on the screen.
///
/// @param x0     - first x coordinate
/// @param y0     - first y coordinate
/// @param x1     - second x coordinate
/// @param y1     - second y coordinate
/// @param color  - color of the line
///
/// @return Nothing.
/// @note Screen zoom factor affects the way the line is drawn. See @ref
///   x3d_screen_draw_pix.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM void x3d_screen_draw_line(int16 x0, int16 y0, int16 x1, int16 y1,
    X3D_Color color);


///////////////////////////////////////////////////////////////////////////////
/// Converts an RGB color to an X3D color.
///
/// @param r  - red component (0 - 255)
/// @param g  - green component (0 - 255)
/// @param b  - blue component (0 - 255)
///
/// @return An X3D_Color created using the current video settings.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM X3D_Color x3d_rgb_to_color(uint8 r, uint8 g, uint8 b);


///////////////////////////////////////////////////////////////////////////////
/// Converts an X3D_Color to an RGB color using the current video settings.
///
/// @param color  - color to convert
/// @param r      - red component dest
/// @param g      - green component dest
/// @param b      - blue component dest
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
X3D_PLATFORM void x3d_color_to_rgb(X3D_Color color, uint8* r, uint8* g,
    uint8* b);


/// @todo Document.
void x3d_screen_draw_line_grad(int16 x0, int16 y0, int16 x1, int16 y1, X3D_Color color0, X3D_Color color1);
void x3d_screen_begin_record(const char* name);
void x3d_screen_record_end(void);

void x3d_screen_draw_scanline_grad(int16 y, int16 left, int16 right, X3D_Color c, fp0x16 scale_left, fp0x16 scale_right, X3D_Color* color_tab, int16 z);
void x3d_screen_zbuf_clear(void);

struct X3D_Span2;

void x3d_screen_draw_scanline_texture(struct X3D_Span2* span, int16 y);
void x3d_screen_zbuf_visualize(void);

void* x3d_screen_get_internal(void);

struct X3D_Texture;

_Bool x3d_platform_screen_load_texture(struct X3D_Texture* tex, const char* file);
void x3d_screen_draw_circle(int16 x0, int16 y0, int16 radius, X3D_Color c);
void x3d_screen_draw_digit(char d, int16 x, int16 y, X3D_Color color);
void x3d_screen_draw_uint32(uint32 num, int16 x, int16 y, X3D_Color c);

void x3d_screen_draw_scanline_color(struct X3D_Span2* span, int16 y, X3D_Color color);

void x3d_screen_set_internal_value(int16 x, int16 y, uint32 val);
uint32 x3d_screen_get_internal_value(int16 x, int16 y);

