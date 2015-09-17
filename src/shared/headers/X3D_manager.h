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

#include "X3D_render.h"               // <--- needed for stack and list
#include "X3D_object.h"
#include "X3D_memory.h"

typedef struct X3D_SegmentManager{
  X3D_List_uint16 segment_offset;
  X3D_Stack segment_data;
} X3D_SegmentManager;

typedef struct X3D_ObjectManager {
  void* object_data __attribute__((depracated));
  X3D_Object* active_list[X3D_MAX_ACTIVE_OBJECTS];
  X3D_ObjectType types[X3D_MAX_OBJECT_TYPES];
  
  X3D_BlockAllocator allocator;

} X3D_ObjectManager;

void x3d_init_segmentmanager(X3D_SegmentManager* state, uint16 max_segments, uint16 seg_pool_size);
void x3d_init_objectmanager(struct X3D_Context* context);

