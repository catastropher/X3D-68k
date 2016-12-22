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

#include "X3D_common.h"

#include "file/X3D_ramfile.h"

void x3d_ramfile_init_empty(X3D_RamFile* file, X3D_RamFileType type, const char* name) {
    file->capacity = 0;
    file->size = 0;
    file->seek_pos = 0;
    file->type = type;
    
    if(name == X3D_RAMFILE_UNNAMED)
        name = "unnamed";
    
    x3d_strncpy(file->name, name, X3D_RAMFILE_MAX_FILE_NAME_LENGTH);
}

static void x3d_ramfile_expand(X3D_RamFile* file, size_t additional_bytes) {
    if(file->size + additional_bytes < file->capacity)
        return;
    
    const size_t CHUNK_SIZE = 512;
    file->capacity = (additional_bytes + CHUNK_SIZE / 2) / CHUNK_SIZE;
    file->data = realloc(file->data, file->capacity);
    
    if(!file->data)
        x3d_log(X3D_ERROR, "Failed to resize ram file!");
}

void x3d_ramfile_write_buf(X3D_RamFile* file, const void* buf, size_t size) {
    x3d_ramfile_expand(file, size);
    memcpy(file, buf, size);
    file->size += size;
}

static inline void x3d_ramfile_write_generic_int(X3D_RamFile* file, uint32 val, size_t total_bytes) {
    x3d_ramfile_expand(file, total_bytes);
    
    for(int i = 0; i < total_bytes; ++i) {
        file->data[file->size++] = val & 0xFF;
        val >>= 8;
    }
}

void x3d_ramfile_write_uint8(X3D_RamFile* file, uint8 byte) {
    x3d_ramfile_write_generic_int(file, byte, 1);
}

void x3d_ramfile_write_int8(X3D_RamFile* file, int8 byte) {
    x3d_ramfile_write_generic_int(file, byte, 1);
}

void x3d_ramfile_write_uint16(X3D_RamFile* file, uint16 val) {
    x3d_ramfile_write_generic_int(file, val, 2);
}

void x3d_ramfile_write_int16(X3D_RamFile* file, int16 val) {
    x3d_ramfile_write_generic_int(file, val, 2);
}

void x3d_ramfile_write_uint32(X3D_RamFile* file, uint32 val) {
    x3d_ramfile_write_generic_int(file, val, 4);
}

void x3d_ramfile_write_int32(X3D_RamFile* file, int32 val) {
    x3d_ramfile_write_generic_int(file, val, 4);
}

void x3d_ramfile_write_string(X3D_RamFile* file, const char* str) {
    x3d_ramfile_write_buf(file, str, strlen(str));
}

void x3d_ramfile_write_length_prefix_string(X3D_RamFile* file, const char* str) {
    
}

void x3d_ramfile_write_vex3d(X3D_RamFile* file, X3D_Vex3D* v) {
    x3d_ramfile_write_int16(file, v->x);
    x3d_ramfile_write_int16(file, v->y);
    x3d_ramfile_write_int16(file, v->z);
}

// Read...

uint8 x3d_ramfile_read_byte(X3D_RamFile* file) {
    if(file->seek_pos >= file->size) {
        x3d_log(X3D_WARN, "Attempt to read past end of file '%s'", file->name);
        return EOF;
    }
    
    return file->data[file->seek_pos++];
}

static inline uint32 x3d_ramfile_read_generic_int(X3D_RamFile* file, size_t total_bytes) {
    uint32 val = 0;
    
    for(int i = 0; i < total_bytes; ++i) {
        val = (val << 8) | x3d_ramfile_read_byte(file);
    }
    
    return val;
}

size_t x3d_ramfile_read_buf(X3D_RamFile* file, void* buf, size_t size) {
    size_t read_size = X3D_MIN(size, file->size - file->seek_pos);
    memcpy(buf, file->data + file->seek_pos, read_size);
    file->seek_pos += read_size;
    
    return read_size;
}

uint8 x3d_ramfile_read_uint8(X3D_RamFile* file) {
    return x3d_ramfile_read_generic_int(file, 1);
}

int8 x3d_ramfile_read_int8(X3D_RamFile* file) {
    return x3d_ramfile_read_generic_int(file, 1);
}

int16 x3d_ramfile_read_int16(X3D_RamFile* file) {
    return x3d_ramfile_read_generic_int(file, 2);
}

uint16 x3d_ramfile_read_uint16(X3D_RamFile* file) {
    return x3d_ramfile_read_generic_int(file, 2);
}

int32 x3d_ramfile_read_int32(X3D_RamFile* file) {
    return x3d_ramfile_read_generic_int(file, 4);
}

uint32 x3d_ramfile_read_uint32(X3D_RamFile* file) {
    return x3d_ramfile_read_generic_int(file, 4);
}

_Bool x3d_ramfile_save_to_file(X3D_RamFile* file, const char* file_name) {
    FILE* f = fopen(file_name, "wb");
    
    if(!f) {
        x3d_log(X3D_ERROR, "Cannot save ramfile '%s' because file '%s' could not be opened for writing", file->name, file_name);
        return X3D_FALSE;
    }
    
    fwrite(f, 1, file->size, f);
    fclose(f);
    
    return X3D_TRUE;
}

