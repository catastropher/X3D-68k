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

#include "X3D_engine.h"
#include "X3D_segment.h"

void write_uint16(FILE* file, uint16 i) {
  fputc(i >> 8, file);
  fputc(i & 0xFF, file);
}

void x3d_save_level(X3D_Context* context, FILE* file) {
  // Segment offset table
  write_uint16(file, context->segment_manager.segment_offset.capacity);
  write_uint16(file, context->segment_manager.segment_offset.size);
  fwrite(context->segment_manager.segment_offset.base, 1, context->segment_manager.segment_offset.size, file);
  
  // Segment data
  write_uint16(file, context->segment_manager.segment_data.size);
  fwrite(context->segment_manager.segment_data.ptr, 1, context->segment_manager.segment_data.size, file);
}

