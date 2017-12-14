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
#include <dirent.h>

#include "X_File.h"
#include "system/X_PackFile.h"
#include "error/X_log.h"
#include "error/X_error.h"
#include "memory/X_alloc.h"
#include "memory/X_String.h"
#include "engine/X_config.h"
#include "util/X_util.h"
#include "math/X_Mat4x4.h"

#define ASSERT_OPEN_FOR_READING(_file) x_assert(x_file_is_open_for_reading(_file), "Attemping to read from file not opened for reading")
#define ASSERT_OPEN_FOR_WRITING(_file) x_assert(x_file_is_open_for_writing(_file), "Attemping to write to file not opened for writing")

static X_String g_searchPaths;
static char g_programPath[256];

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

void x_filesystem_init(const char* programPath)
{
    x_filepath_extract_path(programPath, g_programPath);
    x_string_init(&g_searchPaths, g_programPath);
}

const char* x_filesystem_get_program_path(void)
{
    return g_programPath;
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

static void log_search_paths(void)
{
    char* nextSearchPath = g_searchPaths.data;
    char path[512];
    
    while(get_next_search_path(&nextSearchPath, path))
        x_log_error("    Searched %s", path);
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
        
#ifdef X_FILE_AUTO_ADDED_EXTENSION
        strcat(nextFileToSearch, X_FILE_AUTO_ADDED_EXTENSION);
#endif
        
        file->file = fopen(nextFileToSearch, "rb");
    }
    
    if(!file->file)
    {
        // It's possible the file is a pack file
        x_file_open_from_packfile(file, fileName);
    }
    
    if(!file->file)
    {
        x_log_error("Failed to open file '%s' for reading", fileName);
        log_search_paths();
        return 0;
    }
    
    x_log("Opened file '%s' for reading", fileName);
    file->flags |= X_FILE_OPEN_FOR_READING;
    
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
    
    if(file->flags & X_FILE_OPEN_IN_MEM)
    {
        x_free(file->buffer);
        file->buffer = NULL;
    }
    
    file->flags = 0;
    file->size = 0;
    
    fclose(file->file);
    file->file = NULL;
}

unsigned char* x_file_read_contents(const char* fileName)
{
    X_File file;
    if(!x_file_open_reading(&file, fileName))
        return NULL;
    
    unsigned char* data = x_malloc(file.size);
    if(fread(data, 1, file.size, file.file) != file.size)
        x_system_error("Failed to load %s file contents", fileName);
    
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

x_fp16x16 x_file_read_le_float32_as_fp16x16(X_File* file)
{
    return x_fp16x16_from_float(x_file_read_le_float32(file));
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
    if(fread(dest, 1, bufSize, file->file) != bufSize)
        x_system_error("Failed to write buf to file");
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

void x_file_read_vec3_float_as_fp16x16(X_File* file, X_Vec3* dest)
{
    X_Vec3_float v;
    x_file_read_vec3_float(file, &v);
    *dest = x_vec3_float_to_vec3(&v);
}

void x_file_read_vec2(X_File* file, X_Vec2* dest)
{
    dest->x = x_file_read_le_int32(file);
    dest->y = x_file_read_le_int32(file);
}

void x_file_read_mat4x4(X_File* file, X_Mat4x4* mat)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            mat->elem[i][j] = x_file_read_le_int32(file);
        }
    }
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

void x_file_write_le_int32(X_File* file, int val)
{
    ASSERT_OPEN_FOR_WRITING(file);
    
    for(int i = 0; i < 4; ++i)
        fputc((val >> (i * 8)) & 0xFF, file->file);
}

void x_file_write_vec3(X_File* file, X_Vec3* v)
{
    x_file_write_le_int32(file, v->x);
    x_file_write_le_int32(file, v->y);
    x_file_write_le_int32(file, v->z);
}

void x_file_write_mat4x4(X_File* file, X_Mat4x4* mat)
{
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            x_file_write_le_int32(file, mat->elem[i][j]);
        }
    }
}

void x_file_write_buf(X_File* file, int bufSize, void* src)
{
    ASSERT_OPEN_FOR_WRITING(file);
    fwrite(src, 1, bufSize, file->file);
}

void x_filepath_set_default_file_extension(char* filePath, const char* defaultExtension)
{
    char* str = filePath + strlen(filePath) - 1;
    while(str != filePath && *str != '/')
    {
        if(*str == '.')
            return;
        
        --str;
    }
    
    strcat(filePath, defaultExtension);
}

void x_filepath_extract_path(const char* filePath, char* path)
{
    const char* str = filePath + strlen(filePath) - 1;
    while(str != filePath && *str != '/')
    {
        --str;
    }
    
    while(filePath < str)
    {
        *path++ = *filePath++;
    }
    
    *path = '\0';
}

void x_filepath_extract_filename(const char* filePath, char* fileName)
{
    const char* str = filePath + strlen(filePath) - 1;
    while(str >= filePath && *str != '/')
        --str;
    
    strcpy(fileName, str + 1);
}

void x_filepath_extract_extension(const char* filePath, char* extension)
{
    const char* str = filePath + strlen(filePath) - 1;
    
    while(str != filePath && *str != '/')
    {
        if(*str == '.')
        {
            strcpy(extension, str + 1);
            return;
        }
        
        --str;
    }
    
    *extension = '\0';
}

_Bool x_directoryiterator_open(X_DirectoryIterator* iter, const char* path)
{
    iter->directory = opendir(path);
    iter->searchExtension = NULL;
    
    if(!iter->directory)
    {
        x_log_error("Failed to open directory iterator %s\n", path);
        return 0;
    }
    
    return 1;
}

void x_directoryiterator_set_search_extension(X_DirectoryIterator* iter, const char* searchExtension)
{
    iter->searchExtension = searchExtension;
}

_Bool x_directoryiterator_read_next(X_DirectoryIterator* iter, char* nextFileDest)
{
    struct dirent* entry;
    char fileExtension[256];
    _Bool matchesExtension = 1;
    
    do
    {
        entry = readdir(iter->directory);
        
        if(entry == NULL)
            return 0;
        
        if(iter->searchExtension != NULL)
        {
            x_filepath_extract_extension(entry->d_name, fileExtension);
            matchesExtension = strcmp(fileExtension, iter->searchExtension) == 0;
        }
        
    } while(!matchesExtension);
    
    strcpy(nextFileDest, entry->d_name);
    return 1;
}

void x_directoryiterator_close(X_DirectoryIterator* iter)
{
    if(iter->directory)
        closedir(iter->directory);
}

static _Bool search_packfiles_in_directory(const char* directory, const char* fileToFind, char** fileContents, size_t* fileContentsSize)
{
    X_DirectoryIterator iter;
    if(!x_directoryiterator_open(&iter, directory))
        return 0;
    
    x_directoryiterator_set_search_extension(&iter, "pak");
    
    char packFileName[256];
    _Bool found = 0;
    
    while(!found && x_directoryiterator_read_next(&iter, packFileName))
    {
        X_PackFile packFile;
        if(!x_packfile_read_from_file(&packFile, packFileName))
            continue;
        
        X_PackFileEntry* foundFile = x_packfile_find_file(&packFile, fileToFind);
        if(foundFile != NULL)
        {
            *fileContents = x_packfile_load_file(&packFile, foundFile->name);
            *fileContentsSize = foundFile->size;
            found = 1;
        }
        
        x_packfile_cleanup(&packFile);
    }
    
    x_directoryiterator_close(&iter);
    
    return found;
}

_Bool x_file_open_from_packfile(X_File* file, const char* fileName)
{
    char nextFileToSearch[512];
    char* nextSearchPath = g_searchPaths.data;
    char* fileContents = NULL;
    size_t fileContentsSize;
    
    while(!file->file && get_next_search_path(&nextSearchPath, nextFileToSearch))
    {
        if(search_packfiles_in_directory(nextFileToSearch, fileName, &fileContents, &fileContentsSize))
        {
            file->file = fmemopen(fileContents, fileContentsSize, "r");
            
            if(!file->file)
                return 0;
            
            file->buffer = fileContents;
            file->flags = X_FILE_OPEN_FOR_READING | X_FILE_OPEN_IN_MEM;
            
            return 1;
        }
    }
    
    return 0;
}

