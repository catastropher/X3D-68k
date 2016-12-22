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

#pragma once

#include "X3D_common.h"

#define X3D_RAMFILE_MAX_FILE_NAME_LENGTH 32
#define X3D_RAMFILE_UNNAMED NULL

typedef enum X3D_RamFileType {
    X3D_RAMFILE_LEVEL,
    X3D_RAMFILE_TEXTURE
} X3D_RamFileType;

typedef struct X3D_RamFile {
    char name[X3D_RAMFILE_MAX_FILE_NAME_LENGTH];
    X3D_RamFileType type;
    size_t size;
    size_t capacity;
    uint8* data;
    size_t seek_pos;
} X3D_RamFile;

void x3d_ramfile_write_buf(X3D_RamFile* file, const void* buf, size_t size);
void x3d_ramfile_write_uint8(X3D_RamFile* file, uint8 byte);
void x3d_ramfile_write_int8(X3D_RamFile* file, int8 byte);
void x3d_ramfile_write_uint16(X3D_RamFile* file, uint16 val);
void x3d_ramfile_write_int16(X3D_RamFile* file, int16 val);
void x3d_ramfile_write_uint32(X3D_RamFile* file, uint32 val);
void x3d_ramfile_write_int32(X3D_RamFile* file, int32 val);
void x3d_ramfile_write_string(X3D_RamFile* file, const char* str);

uint8 x3d_ramfile_read_uint8(X3D_RamFile* file);
int8 x3d_ramfile_read_int8(X3D_RamFile* file);
int16 x3d_ramfile_read_int16(X3D_RamFile* file);
uint16 x3d_ramfile_read_uint16(X3D_RamFile* file);
int32 x3d_ramfile_read_int32(X3D_RamFile* file);
uint32 x3d_ramfile_read_uint32(X3D_RamFile* file);

_Bool x3d_ramfile_save_to_file(X3D_RamFile* file, const char* file_name);

static inline size_t x3d_ramfile_size(const X3D_RamFile* file) {
    return file->size;
}

static inline void x3d_ramfile_set_seek_pos(X3D_RamFile* file, size_t seek_pos) {
    if(seek_pos < file->capacity)
        file->seek_pos = seek_pos;
}

