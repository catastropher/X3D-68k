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

#include "X3D_common.h"

#include "file/X3D_buffer.h"

void x3d_buffer_init_empty(X3D_Buffer* buf) {
    buf->capacity = 0;
    buf->size = 0;
    buf->seek_pos = 0;
    buf->data = NULL;
}

void x3d_buffer_init_existing(X3D_Buffer* buf, void* data, size_t size, int id) {
    buf->data = data;
    buf->size = size;
    buf->capacity = size;
    buf->id = id;
    buf->seek_pos = 0;
}

static void x3d_buffer_expand(X3D_Buffer* buf, size_t additional_bytes) {
    if(buf->size + additional_bytes < buf->capacity)
        return;
    
    const size_t CHUNK_SIZE = 512;
    buf->capacity = (additional_bytes + CHUNK_SIZE / 2) / CHUNK_SIZE;
    buf->data = realloc(buf->data, buf->capacity);
    
    if(!buf->data)
        x3d_log(X3D_ERROR, "Failed to resize ram buf!");
}

void x3d_buffer_write_data(X3D_Buffer* buf, const void* data, size_t size) {
    x3d_buffer_expand(buf, size);
    memcpy(buf->data + buf->size, data, size);
    buf->size += size;
}

static inline void x3d_buffer_write_generic_int(X3D_Buffer* buf, uint32 val, size_t total_bytes) {
    x3d_buffer_expand(buf, total_bytes);
    
    for(int i = 0; i < total_bytes; ++i) {
        buf->data[buf->size++] = val & 0xFF;
        val >>= 8;
    }
}

void x3d_buffer_write_uint8(X3D_Buffer* buf, uint8 byte) {
    x3d_buffer_write_generic_int(buf, byte, 1);
}

void x3d_buffer_write_int8(X3D_Buffer* buf, int8 byte) {
    x3d_buffer_write_generic_int(buf, byte, 1);
}

void x3d_buffer_write_uint16(X3D_Buffer* buf, uint16 val) {
    x3d_buffer_write_generic_int(buf, val, 2);
}

void x3d_buffer_write_int16(X3D_Buffer* buf, int16 val) {
    x3d_buffer_write_generic_int(buf, val, 2);
}

void x3d_buffer_write_uint32(X3D_Buffer* buf, uint32 val) {
    x3d_buffer_write_generic_int(buf, val, 4);
}

void x3d_buffer_write_int32(X3D_Buffer* buf, int32 val) {
    x3d_buffer_write_generic_int(buf, val, 4);
}

void x3d_buffer_write_string(X3D_Buffer* buf, const char* str) {
    x3d_buffer_write_data(buf, str, strlen(str));
}

void x3d_buffer_write_length_prefix_string(X3D_Buffer* buf, const char* str) {
    
}

void x3d_buffer_write_vex3d(X3D_Buffer* buf, X3D_Vex3D* v) {
    x3d_buffer_write_int16(buf, v->x);
    x3d_buffer_write_int16(buf, v->y);
    x3d_buffer_write_int16(buf, v->z);
}

// Read...

uint8 x3d_buffer_read_byte(X3D_Buffer* buf) {
    if(buf->seek_pos >= buf->size) {
        x3d_log(X3D_WARN, "Attempt to read past end of buf");
        return EOF;
    }
    
    return buf->data[buf->seek_pos++];
}

static inline uint32 x3d_buffer_read_generic_int(X3D_Buffer* buf, size_t total_bytes) {
    uint32 val = 0;
    
    for(int i = 0; i < total_bytes; ++i) {
        val = (val << 8) | x3d_buffer_read_byte(buf);
    }
    
    return val;
}

size_t x3d_buffer_read_data(X3D_Buffer* buf, void* dest, size_t size) {
    size_t read_size = X3D_MIN(size, buf->size - buf->seek_pos);
    memcpy(dest, buf->data + buf->seek_pos, read_size);
    buf->seek_pos += read_size;
    
    return read_size;
}

uint8 x3d_buffer_read_uint8(X3D_Buffer* buf) {
    return x3d_buffer_read_generic_int(buf, 1);
}

int8 x3d_buffer_read_int8(X3D_Buffer* buf) {
    return x3d_buffer_read_generic_int(buf, 1);
}

int16 x3d_buffer_read_int16(X3D_Buffer* buf) {
    return x3d_buffer_read_generic_int(buf, 2);
}

uint16 x3d_buffer_read_uint16(X3D_Buffer* buf) {
    return x3d_buffer_read_generic_int(buf, 2);
}

int32 x3d_buffer_read_int32(X3D_Buffer* buf) {
    return x3d_buffer_read_generic_int(buf, 4);
}

uint32 x3d_buffer_read_uint32(X3D_Buffer* buf) {
    return x3d_buffer_read_generic_int(buf, 4);
}

_Bool x3d_buffer_save_to_buf(X3D_Buffer* buf, const char* buf_name) {
    FILE* f = fopen(buf_name, "wb");
    
    if(!f)
        return X3D_FALSE;
    
    fwrite(f, 1, buf->size, f);
    fclose(f);
    
    return X3D_TRUE;
}

