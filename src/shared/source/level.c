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

uint16 read_uint16(FILE* file) {
  uint16 i = (uint8)fgetc(file) << 8;
  
  return i | fgetc(file);
}

void x3d_save_level(X3D_Context* context, FILE* file) {
  // Segment offset table
  printf("Cap %u\n", context->segment_manager.segment_offset.capacity);
  
  write_uint16(file, context->segment_manager.segment_offset.capacity);
  write_uint16(file, context->segment_manager.segment_offset.size);
  fwrite(context->segment_manager.segment_offset.base, 1, context->segment_manager.segment_offset.size * 2, file);
  
  // Segment data
  
  X3D_Stack* s = &context->segment_manager.segment_data;
  
  uint16 size = s->base + s->size - s->ptr;
  
  printf("Size: %d\n", size);
  
  write_uint16(file, size);
  fwrite(s->ptr, 1, size, file);
  
  // Key data
  write_uint16(file, context->record);
  
  if(context->record) {
    write_uint16(file, context->key_data_size);
    fwrite(context->key_data, 2, context->key_data_size, file);
  }
}

void x3d_load_level(X3D_Context* context, FILE* file) {
  context->segment_manager.segment_offset.capacity = read_uint16(file);
  
  printf("Cap %u\n", context->segment_manager.segment_offset.capacity);
  
  context->segment_manager.segment_offset.size = read_uint16(file);
  fread(context->segment_manager.segment_offset.base, 1, context->segment_manager.segment_offset.size * 2, file);
  
  
  X3D_Stack* s = &context->segment_manager.segment_data;
  
  uint16 size = read_uint16(file);
  
  s->ptr = s->base + s->size - size;
  
  fread(s->ptr, 1, size, file);
  
  if(read_uint16(file)) {
    // Load key data
    context->key_data = malloc(10000);
    context->key_data_size = read_uint16(file);
    fread(context->key_data, 2, context->key_data_size, file);
    printf("Key frames: %d\n", context->key_data_size);
  }
}