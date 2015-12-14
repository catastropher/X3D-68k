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

#include "X3D_platform.h"

// @todo: move to better location
enum ErrorCodes {
  ERROR_NOT_IMPLEMENTED
};


#define NOT_IMPLEMENTED() x3d->error.throw_error(ERROR_NOT_IMPLEMENTED, "__FUNCTION__  is not implemented")

typedef struct X3D_StatusBarInterface {
	void (*add)(const char*);
} X3D_StatusBarInterface;

typedef struct X3D_ErrorInterface {
  char msg[128];
  uint16 id;
  void (*throw_error)(uint16 id, const char* format, ...);
} X3D_ErrorInterface;

struct X3D_FreeList;

typedef struct X3D_FreeListInterface {
  void (*create)(struct X3D_FreeList* list, void* mem, size_t mem_size, uint16 block_size);
  void* (*alloc_block)(struct X3D_FreeList* list);
  void (*free_block)(struct X3D_FreeList* list, void* block);
  void (*reset)(struct X3D_FreeList* list);
} X3D_FreeListInterface;

typedef struct X3D_Interface {
  X3D_Screen screen;
  
  X3D_FreeListInterface freelist;
  X3D_StatusBarInterface status;
  X3D_ErrorInterface error;
} X3D_Interface;

extern X3D_Interface x3d_global_enginestate;

register X3D_Interface* x3d asm("a4");



