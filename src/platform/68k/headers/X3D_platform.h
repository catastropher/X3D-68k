/* This file is part of X3D.
 *
 * X3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * X3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with X3D. If not, see <http://www.gnu.org/licenses/>.
 */



#pragma once

#include <tigcclib.h>

//=============================================================================
// Defines
//=============================================================================

#define x3d_main(...) _main()

/// Maximum value a 16-bit integer can hold
#ifndef INT16_MAX
#define INT16_MAX 32767
#endif

//=============================================================================
// Types
//=============================================================================

/// An 8-bit signed integer
typedef char int8;

/// An 8-bit unsigned integer
typedef unsigned char uint8;

/// A 16-bit signed integer
typedef short int16;

/// A 16-bit unsigned integer
typedef unsigned short uint16;

/// A 32-bit signed integer
typedef long int32;

/// A 32-bit unsigned integer
typedef unsigned long uint32;

/// A 64-bit signed integer
typedef long long int64;

/// A 64-bit unsigned integer
typedef unsigned long long uint64;

enum {
  X3D_FALSE = 0,
  X3D_TRUE = 1
};

typedef struct X3D_Screen {
  int16 width;
  int16 height;
  uint8 bpp;
  uint8 flags;
  
  LCD_BUFFER* save_screen;
} X3D_Screen;

enum {
  X3D_SPLIT_PLANE_GRAYSCALE = 1
};


static inline _Bool x3d_platform_screen_init(X3D_Screen* screen, int16 width, int16 height, int16 bpp) {
  screen->save_screen = malloc(LCD_SIZE);
  
  if(!screen->save_screen)
    return X3D_FALSE;
  
  LCD_save(screen->save_screen);
  
  screen->width = width;
  screen->height = height;
  screen->bpp = bpp;
  screen->flags = X3D_SPLIT_PLANE_GRAYSCALE;
  
  return X3D_TRUE;
}

static inline void x3d_platform_screen_cleanup(X3D_Screen* screen) {
  LCD_restore(screen->save_screen);
  free(screen->save_screen);
}











