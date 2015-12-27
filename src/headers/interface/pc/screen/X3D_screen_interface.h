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
// X3D_screen_interface.h -> screen functionality

#pragma once

#include "common/X3D_interface.h"
#include "common/X3D_int.h"

#include "platform/screen/X3D_platform_screen.h"

/**
 * Flips the double buffer - anything that was drawn to the screen buffer will
 *    now be visible on the screen.
 * 
 * @return Nothing.
 */
X3D_INTERFACE static inline void x3d_screen_flip() {
  x3dplatform_screen_flip();
}

/**
 * Draws a pixel to the screen buffer.
 * 
 * @param x     - x coordinate
 * @param y     - y coordinate
 * @param color - color of the pixel
 * 
 * @return Nothing.
 */
X3D_INTERFACE static inline void x3d_draw_pix(int16 x, int16 y, X3D_Color color) {
  x3dplatform_draw_pix(x, y, color);
}

/**
 * Draws a line to the screen buffer.
 * 
 * @param x0    - first x coordinate
 * @param y0    - first y coordinate
 * @param x1    - second x coordinate
 * @param y2    - second y coordinate
 * @param color - color of the line
 * 
 * @return Nothing.
 */
X3D_INTERFACE static inline void x3d_draw_line(int16 x0, int16 y0, int16 x1,
                                               int16 y1, X3D_Color color) {
  x3dplatform_draw_line(x0, y0, x1, y1, color);
}

/**
 * Converts an RGB color to the platform-dependent representation of color
 *    (@ref X3D_Color).
 * 
 * @param r   - red component (0 - 255)
 * @param g   - blue component (0 - 255)
 * @param b   - green component (0 - 255)
 * 
 * @return The X3D_Color representation of the RGB color (r, g, b).
 */
X3D_INTERFACE static inline X3D_Color x3d_rgb_to_color(uint8 r, uint8 g, uint8 b) {
  x3dplatform_rgb_to_color(r, g, b);
}

/**
 * Converts an X3D color to an RGB color.
 * 
 * @param color   - color to convert
 * @param r       - red component destination
 * @param g       - green component destination
 * @param b       - blue component destination
 * 
 * @return Nothing.
 */
X3D_INTERFACE static inline void x3d_color_to_rgb(X3D_Color color, uint8* r, uint8* g, uint8* b) {
  x3dplatform_color_to_rgb(color, r, g, b);
}

