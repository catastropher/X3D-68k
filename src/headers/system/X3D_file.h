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

#include <stdio.h>

#include "X3D_common.h"

static inline int32 x3d_file_read_int32(FILE* file) {
    int val;
    fscanf(file, "%d", &val);
    return val;
}

static inline X3D_Vex3D x3d_file_read_vex3d(FILE* file) {
    int x, y, z;
    fscanf(file, "%d %d %d", &x, &y, &z);
    
    return x3d_vex3d_make(x, y, z);
}

static inline int32 x3d_file_read_little_endian_int32(FILE* file) {
    uint32 val = 0;
    
    for(int i = 0; i < 4; ++i)
        val = val | ((uint32)fgetc(file) << (8 * i));
    
    return val;
}

static inline void x3d_file_write_little_endian_int32(FILE* file, uint32 val) {
    for(int i = 0; i < 4; ++i) {
        fputc(val & 0xFF, file);
        val >>= 8;
    }
}

static inline void x3d_file_write_buf(FILE* file, void* buf, size_t size) {
    if(buf)
        fwrite(buf, 1, size, file);
}

static inline void x3d_file_read_buf(FILE* file, void* dest, size_t size) {
    if(dest)
        fread(dest, 1, size, file);
}

static inline void x3d_file_read_string(FILE* file, char* dest, size_t chars_to_read) {
    fread(dest, 1, chars_to_read, file);
    dest[chars_to_read] = '\0';
}

static inline void x3d_file_seek(FILE* file, size_t pos) {
    fseek(file, pos, SEEK_SET);
}


