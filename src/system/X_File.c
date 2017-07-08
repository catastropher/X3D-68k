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

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "X_File.h"
#include "error/X_log.h"
#include "error/X_error.h"
#include "memory/X_alloc.h"
#include "memory/X_String.h"

#define ASSERT_OPEN_FOR_READING(_file) x_assert(x_file_is_open_for_reading(_file), "Attemping to read from file not opened for reading")
#define ASSERT_OPEN_FOR_WRITING(_file) x_assert(x_file_is_open_for_writing(_file), "Attemping to write to file not opened for writing")

static X_String g_searchPaths;

static inline void determine_file_size(X_File* file)
{
    fseek(file->file, 0, SEEK_END);
    file->size = ftell(file->file);
    rewind(file->file);
}

static _Bool get_next_search_path(char** start, char* dest)
{
    char* path = *start;
    
    if(*path == '\0')
        return 0;
    
    while(*path && *path != ';')
        *dest++ = *path++;
    
    *dest = '\0';
    
    if(*path == ';')
        ++path;
    
    *start = path;
    
    return 1;
}

void x_filesystem_init(void)
{
    x_string_init(&g_searchPaths, ".");
}

void x_filesystem_cleanup(void)
{
    x_string_cleanup(&g_searchPaths);
}

void x_filesystem_add_search_path(const char* searchPath)
{
    x_string_concat_cstr(&g_searchPaths, ";");
    x_string_concat_cstr(&g_searchPaths, searchPath);
}

_Bool x_file_open_reading(X_File* file, const char* fileName)
{
    file->flags = 0;
    file->size = 0;
    file->file = NULL;
    
    char nextFileToSearch[512];
    char* nextSearchPath = g_searchPaths.data;
    
    while(!file->file && get_next_search_path(&nextSearchPath, nextFileToSearch))
    {
        strcat(nextFileToSearch, "/");
        strcat(nextFileToSearch, fileName);
        file->file = fopen(nextFileToSearch, "rb");
    }
    
    if(!file->file)
    {
        x_log_error("Failed to open file '%s' for reading", fileName);
        return 0;
    }
    
    x_log("Opened file '%s' for reading", fileName);
    file->flags = X_FILE_OPEN_FOR_READING;
    determine_file_size(file);
    
    return 1;
}

void x_file_close(X_File* file)
{
    if(!x_file_is_open(file))
    {
        x_log_error("Attempting to close unopened file");
        return;
    }
    
    fclose(file->file);
    file->file = NULL;
    file->flags = 0;
    file->size = 0;
}

unsigned char* x_file_read_contents(const char* fileName)
{
    X_File file;
    if(!x_file_open_reading(&file, fileName))
        return NULL;
    
    unsigned char* data = x_malloc(file.size);
    fread(data, 1, file.size, file.file);
    
    x_file_close(&file);
    return data;
}

int x_file_read_char(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    return fgetc(file->file);
}

_Bool x_file_read_line(X_File* file, int maxLineLength, char* line)
{
    if(feof(file->file))
        return 0;
    
    char* lineEnd = line + maxLineLength - 1;   // Save room for null terminator
    int c;
    
    while((c = fgetc(file->file)) != EOF)
    {
        if(c == '\n')
            break;
        
        x_assert(line < lineEnd, "Trying to read line that's too long from file");
        
        *line++ = c;
    }
    
    *line = '\0';
    return 1;
}

void x_file_read_cstr(X_File* file, char* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    
    int c;
    while(!feof(file->file) && (c = fgetc(file->file)) != '\0')
        *dest++ = c;
    
    *dest = '\0';
}

int x_file_read_le_int32(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    
    unsigned int val = 0;
    for(int i = 0; i < 4; ++i)
        val |= (unsigned int)fgetc(file->file) << (i * 8);
    
    return val;
}

int x_file_read_le_int16(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    
    unsigned int val = 0;
    for(int i = 0; i < 2; ++i)
        val |= fgetc(file->file) << (i * 8);
    
    return val;
}

float x_file_read_le_float32(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    
    x_assert(sizeof(float) == 4, "Float size is not 4");
    x_assert(sizeof(int) == 4, "Int size is not 4");
    
    union {
        int i;
        float f;
    } converter;
    
    converter.i = x_file_read_le_int32(file);
    return converter.f;
}

int x_file_read_be_int32(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    
    unsigned int val = 0;
    for(int i = 3; i >= 0; --i)
        val |= fgetc(file->file) << (i * 8);
    
    return val;
}

int x_file_read_be_int16(X_File* file)
{
    ASSERT_OPEN_FOR_READING(file);
    
    unsigned int val = 0;
    for(int i = 1; i >= 0; --i)
        val |= fgetc(file->file) << (i * 8);
    
    return val;
}

void x_file_seek(X_File* file, size_t pos)
{
    x_assert(x_file_is_open(file), "Trying to seek on unopened file");
    fseek(file->file, pos, SEEK_SET);
}

void x_file_read_fixed_length_str(X_File* file, int strLength, char* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    
    dest += fread(dest, 1, strLength, file->file);
    *dest = '\0';
}

void x_file_read_buf(X_File* file, int bufSize, void* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    fread(dest, 1, bufSize, file->file);
}

void x_file_read_vec3(X_File* file, X_Vec3* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    dest->x = x_file_read_le_int32(file);
    dest->y = x_file_read_le_int32(file);
    dest->z = x_file_read_le_int32(file);
}

void x_file_read_vec3_float(X_File* file, X_Vec3_float* dest)
{
    ASSERT_OPEN_FOR_READING(file);
    dest->x = x_file_read_le_float32(file);
    dest->y = x_file_read_le_float32(file);
    dest->z = x_file_read_le_float32(file);
}

_Bool x_file_open_writing(X_File* file, const char* fileName)
{
    file->file = fopen(fileName, "wb");
    file->flags = 0;
    file->size = 0;
    
    if(!file->file)
    {
        x_log_error("Failed to open file '%s' for reading", fileName);
        return 0;
    }
    
    x_log("Opened file '%s' for writing", fileName);
    file->flags = X_FILE_OPEN_FOR_WRITING;
    
    return 1;
}

static _Bool create_path_for_file(const char* filePath) {
    char fileName[512];
    const int MODE = 0755;

    strncpy(fileName, filePath, sizeof(fileName));

    for(char* dirStart = strchr(fileName + 1, '/'); dirStart != NULL; dirStart = strchr(dirStart + 1, '/'))
    {
        *dirStart = '\0';

        if(mkdir(fileName, MODE) == -1)
        {
            if(errno != EEXIST)
            {
                *dirStart = '/';
                return 0;
            }
        }
        
        *dirStart = '/';
    }
    
    return 1;
}

_Bool x_file_open_writing_create_path(X_File* file, const char* fileName)
{
    if(!create_path_for_file(fileName))
    {
        x_log_error("Failed to create path for %s", fileName);
        return 0;
    }
    
    return x_file_open_writing(file, fileName);
}

void x_file_write_le_int16(X_File* file, int val)
{
    ASSERT_OPEN_FOR_WRITING(file);
    
    for(int i = 0; i < 2; ++i)
        fputc((val >> (i * 8)) & 0xFF, file->file);
}

void x_file_write_buf(X_File* file, int bufSize, void* src)
{
    ASSERT_OPEN_FOR_WRITING(file);
    fwrite(src, 1, bufSize, file->file);
}




