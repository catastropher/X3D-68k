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
#include <dirent.h>

#include "geo/X_Vec3.h"
#include "geo/X_Vec2.h"

// TODO: this should be used in place of magic numbers
#define X_FILENAME_MAX_LENGTH 256

typedef enum X_FileFlags
{
    X_FILE_OPEN_FOR_READING = 1,
    X_FILE_OPEN_FOR_WRITING = 2,
    X_FILE_OPEN_IN_MEM = 4
} X_FileFlags;

typedef struct X_File
{
    FILE* file;
    size_t size;
    X_FileFlags flags;
    void* buffer;
} X_File;

typedef struct X_DirectoryIterator
{
    DIR* directory;
    const char* searchExtension;
} X_DirectoryIterator;

void x_filesystem_init(const char* programPath);
void x_filesystem_cleanup(void);
const char* x_filesystem_get_program_path(void);

void x_filesystem_add_search_path(const char* searchPath);

_Bool x_file_open_reading(X_File* file, const char* fileName);
void x_file_close(X_File* file);
unsigned char* x_file_read_contents(const char* fileName);
int x_file_read_char(X_File* file);
_Bool x_file_read_line(X_File* file, int maxLineLength, char* line);
void x_file_read_cstr(X_File* file, char* dest);
void x_file_read_buf(X_File* file, int bufSize, void* dest);
int x_file_read_le_int32(X_File* file);
int x_file_read_be_int32(X_File* file);

int x_file_read_be_int16(X_File* file);
int x_file_read_le_int16(X_File* file);

float x_file_read_le_float32(X_File* file);
x_fp16x16 x_file_read_le_float32_as_fp16x16(X_File* file);

void x_file_seek(X_File* file, size_t pos);
void x_file_read_fixed_length_str(X_File* file, int strLength, char* dest);
void x_file_read_vec3(X_File* file, X_Vec3* dest);
void x_file_read_vec3_float(X_File* file, X_Vec3_float* dest);
void x_file_read_vec3_float_as_fp16x16(X_File* file, X_Vec3_fp16x16* dest);

void x_file_read_vec2(X_File* file, X_Vec2* dest);

_Bool x_file_open_writing(X_File* file, const char* fileName);
_Bool x_file_open_writing_create_path(X_File* file, const char* fileName);

void x_file_write_le_int16(X_File* file, int val);
void x_file_write_buf(X_File* file, int bufSize, void* src);
void x_file_write_le_int32(X_File* file, int val);
void x_file_write_vec3(X_File* file, X_Vec3* v);

struct X_Mat4x4;
void x_file_write_mat4x4(X_File* file, struct X_Mat4x4* mat);

void x_filepath_set_default_file_extension(char* filePath, const char* defaultExtension);
void x_filepath_extract_path(const char* filePath, char* path);
void x_filepath_extract_filename(const char* filePath, char* fileName);
void x_filepath_extract_extension(const char* filePath, char* extension);

_Bool x_file_open_from_packfile(X_File* file, const char* fileName);

_Bool x_directoryiterator_open(X_DirectoryIterator* iter, const char* path);
void x_directoryiterator_set_search_extension(X_DirectoryIterator* iter, const char* searchExtension);
_Bool x_directoryiterator_read_next(X_DirectoryIterator* iter, char* nextFileDest);
void x_directoryiterator_close(X_DirectoryIterator* iter);

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

