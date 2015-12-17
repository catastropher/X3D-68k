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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

//=============================================================================
// Types
//=============================================================================

/// An 8-bit signed integer
typedef int8_t int8;

/// An 8-bit unsigned integer
typedef uint8_t uint8;

/// A 16-bit signed integer
typedef int16_t int16;

/// A 16-bit unsigned integer
typedef uint16_t uint16;

/// A 32-bit signed integer
typedef int32_t int32;

/// A 32-bit unsigned integer
typedef uint32_t uint32;

/// A 64-bit signed integer
typedef int64_t int64;

/// A 64-bit unsigned integer
typedef uint64_t uint64;



enum {
  X3D_FALSE = 0,
  X3D_TRUE = 1
};

typedef struct X3D_Screen {
  int16 width;
  int16 height;
  uint8 bpp;
  uint8 flags;
} X3D_Screen;

enum {
  X3D_SPLIT_PLANE_GRAYSCALE = 1
};

struct X3D_Interface;
extern struct X3D_Interface* x3d;

#define X3D_MAX_TRY_DEPTH 16

typedef struct X3D_ErrorFrame {
  jmp_buf buf;
  struct X3D_ErrorFrame* next;
} X3D_ErrorFrame;

typedef struct X3D_PlatformData {
  struct {
    X3D_ErrorFrame* head;
  } error_data;
} X3D_PlatformData;


#include "X3D_platform_init.h"