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
#include <stdlib.h>

#include "geo/X_Vec3.h"

typedef enum X_FileFlags
{
    X_FILE_OPEN_FOR_READING = 1,
    X_FILE_OPEN_FOR_WRITING = 2
} X_FileFlags;

typedef struct X_File
{
    FILE* file;
    size_t size;
    X_FileFlags flags;
} X_File;

_Bool x_file_open_reading(X_File* file, const char* fileName);
void x_file_close(X_File* file);
unsigned char* x_file_read_contents(const char* fileName);
int x_file_read_char(X_File* file);
void x_file_read_cstr(X_File* file, char* dest);
void x_file_read_buf(X_File* file, int bufSize, char* dest);
int x_file_read_le_int32(X_File* file);
int x_file_read_be_int32(X_File* file);

int x_file_read_be_int16(X_File* file);
int x_file_read_le_int16(X_File* file);

void x_file_seek(X_File* file, size_t pos);
void x_file_read_fixed_length_str(X_File* file, int strLength, char* dest);
void x_file_read_vec3(X_File* file, X_Vec3* dest);

_Bool x_file_open_writing(X_File* file, const char* fileName);
void x_file_write_le_int16(X_File* file, int val);
void x_file_write_buf(X_File* file, int bufSize, const char* src);

static inline _Bool x_file_is_open_for_reading(const X_File* file)
{
    return file->flags & X_FILE_OPEN_FOR_READING;
}

static inline _Bool x_file_is_open_for_writing(const X_File* file)
{
    return file->flags & X_FILE_OPEN_FOR_WRITING;
}

static inline _Bool x_file_is_open(const X_File* file)
{
    return x_file_is_open_for_reading(file) || x_file_is_open_for_writing(file);
}

