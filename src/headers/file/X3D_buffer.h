// This buf is part of X3D.
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

#define X3D_RAMFILE_MAX_FILE_NAME_LENGTH 32
#define X3D_RAMFILE_UNNAMED NULL


typedef struct X3D_Buffer {
    size_t size;
    size_t capacity;
    uint8* data;
    size_t seek_pos;
    int id;
} X3D_Buffer;

void x3d_buffer_init_empty(X3D_Buffer* buf);
void x3d_buffer_init_existing(X3D_Buffer* buf, void* data, size_t size, int id);

void x3d_buffer_write_data(X3D_Buffer* buf, const void* data, size_t size);
void x3d_buffer_write_uint8(X3D_Buffer* buf, uint8 byte);
void x3d_buffer_write_int8(X3D_Buffer* buf, int8 byte);
void x3d_buffer_write_uint16(X3D_Buffer* buf, uint16 val);
void x3d_buffer_write_int16(X3D_Buffer* buf, int16 val);
void x3d_buffer_write_uint32(X3D_Buffer* buf, uint32 val);
void x3d_buffer_write_int32(X3D_Buffer* buf, int32 val);
void x3d_buffer_write_string(X3D_Buffer* buf, const char* str);

size_t x3d_buffer_read_data(X3D_Buffer* buf, void* dest, size_t size);
uint8 x3d_buffer_read_uint8(X3D_Buffer* buf);
int8 x3d_buffer_read_int8(X3D_Buffer* buf);
int16 x3d_buffer_read_int16(X3D_Buffer* buf);
uint16 x3d_buffer_read_uint16(X3D_Buffer* buf);
int32 x3d_buffer_read_int32(X3D_Buffer* buf);
uint32 x3d_buffer_read_uint32(X3D_Buffer* buf);

_Bool x3d_buffer_save_to_file(X3D_Buffer* buf, const char* buf_name);

static inline size_t x3d_buffer_size(const X3D_Buffer* buf) {
    return buf->size;
}

static inline void x3d_buffer_set_seek_pos(X3D_Buffer* buf, size_t seek_pos) {
    if(seek_pos < buf->capacity)
        buf->seek_pos = seek_pos;
}

