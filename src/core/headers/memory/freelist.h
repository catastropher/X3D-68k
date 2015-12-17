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

#include "X3D_interface.h"

typedef struct X3D_FreeList X3D_FreeList;

void x3d_freelist_create(X3D_FreeList* list, void* mem, size_t mem_size, uint16 block_size);
void* x3d_freelist_alloc(X3D_FreeList* list);
void x3d_freelist_free(X3D_FreeList* list, void* block);
void x3d_freelist_reset(X3D_FreeList* list);

static inline void x3d_freelist_load_interface() {
	x3d->freelist.create = x3d_freelist_create;
	x3d->freelist.alloc_block = x3d_freelist_alloc;
	x3d->freelist.free_block = x3d_freelist_free;
	x3d->freelist.reset = x3d_freelist_reset;
}


