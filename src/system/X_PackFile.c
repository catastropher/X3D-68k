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

#include <string.h>
#include <strings.h>

#include "X_PackFile.h"
#include "error/X_log.h"
#include "memory/X_alloc.h"

static _Bool x_packfile_read_header(X_PackFile* file)
{
    unsigned int magicNumber = x_file_read_le_int32(&file->file);
    if(magicNumber != X_PACKFILE_HEADER_MAGIC_NUMBER)
    {
        x_log_error("Pack file magic number not 'PACK'");
        return 0;
    }
    
    file->header.fileTableOffset = x_file_read_le_int32(&file->file);
    file->header.fileTableSize = x_file_read_le_int32(&file->file);
    
    return 1;
}

static void x_packfile_read_entry(X_PackFile* file, X_PackFileEntry* entry)
{
    x_file_read_buf(&file->file, X_PACKFILEENTRY_NAME_LENGTH, entry->name);
    entry->fileOffset = x_file_read_le_int32(&file->file);
    entry->size = x_file_read_le_int32(&file->file);
}

static void x_packfile_read_entries(X_PackFile* file)
{
    x_file_seek(&file->file, file->header.fileTableOffset);
    
    file->totalEntries = file->header.fileTableSize / X_PACKFILEENTRY_SIZE;
    file->entries = x_malloc(file->totalEntries * sizeof(X_PackFileEntry));
    
    for(int i = 0; i < file->totalEntries; ++i)
        x_packfile_read_entry(file, file->entries + i);
}

_Bool x_packfile_read_from_file(X_PackFile* file, const char* fileName)
{
    file->totalEntries = 0;
    file->entries = NULL;
    
    if(!x_file_open_reading(&file->file, fileName))
    {
        x_log("Could not open packfile %s", fileName);
        return 0;
    }
    
    if(!x_packfile_read_header(file))
        return 0;
    
    x_packfile_read_entries(file);
    
    return 1;
}

void x_packfile_print_files(X_PackFile* file)
{
    printf("===============Files============\n");
    
    for(int i = 0; i < file->totalEntries; ++i)
        printf("%s\n", file->entries[i].name);
    
    printf("\n");
}

void x_packfile_cleanup(X_PackFile* file)
{
    x_free(file->entries);
    file->entries = NULL;
    file->totalEntries = 0;
    
    if(x_file_is_open(&file->file))
        x_file_close(&file->file);
}

char* x_packfile_load_file(X_PackFile* file, const char* fileName)
{
    for(int i = 0; i < file->totalEntries; ++i)
    {
        X_PackFileEntry* entry = file->entries + i;
        if(strcmp(entry->name, fileName) == 0)
        {
            char* contents = x_malloc(entry->size + 1);
            
            x_file_seek(&file->file, entry->fileOffset);
            x_file_read_buf(&file->file, entry->size, contents);
            contents[entry->size] = '\0';
            
            return contents;
        }
    }
    
    return NULL;
}

_Bool x_packfile_extract(X_PackFile* file, const char* dirToExtractTo)
{
    char fileName[512];
    
    for(int i = 0; i < file->totalEntries; ++i)
    {
        X_PackFileEntry* entry = file->entries + i;
        
        strcpy(fileName, dirToExtractTo);
        strcat(fileName, "/");
        strcat(fileName, entry->name);
        
        X_File entryFile;
        if(!x_file_open_writing_create_path(&entryFile, fileName))
            return 0;
        
        char* contents = x_packfile_load_file(file, entry->name);
        x_file_write_buf(&entryFile, entry->size, contents);
        
        x_file_close(&entryFile);
        x_free(contents);
    }
    
    return 1;
}

X_PackFileEntry* x_packfile_find_file(X_PackFile* file, const char* fileToFind)
{
    for(int i = 0; i < file->totalEntries; ++i)
    {
        char fileName[256];
        x_filepath_extract_filename(file->entries[i].name, fileName);
        
        if(strcasecmp(fileName, fileToFind) == 0)
            return file->entries + i;
    }
    
    return NULL;
}


