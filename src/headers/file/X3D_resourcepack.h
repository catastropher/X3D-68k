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
#include "system/X3D_file.h"
#include "file/X3D_buffer.h"

#define X3D_RESOURCEPACK_FILE_ENTRY_SIZE 64
#define X3D_RESOURCEPACK_MAX_FILENAME_LENGTH 56
#define X3D_RESOURCEPACK_HEADER_SIZE 12

typedef struct X3D_ResourcePackFile {
    char name[X3D_RESOURCEPACK_MAX_FILENAME_LENGTH];
    size_t size;
    size_t data_offset;
    void* loaded_data;
    int ref_count;
} X3D_ResourcePackFile;

typedef struct X3D_ResourcePack {
    FILE* file;
    X3D_ResourcePackFile* pack_files;
    size_t total_files;
} X3D_ResourcePack;

_Bool x3d_resourcepack_load_from_file(X3D_ResourcePack* pack, const char* file_name);
_Bool x3d_resourcepack_open_packfile(X3D_ResourcePack* pack, const char* file_name, X3D_Buffer* dest);
void x3d_resourcepack_close_packfile(X3D_ResourcePack* pack, X3D_Buffer* data_src);
void x3d_resourcepack_close_all_packfiles(X3D_ResourcePack* pack);
void x3d_resourcepack_cleanup(X3D_ResourcePack* pack);
_Bool x3d_resourcepack_save_packfiles_to_file(X3D_ResourcePackFile* pack_files, int total_files, const char* file_name);

void x3d_resourcepack_print_file_header(X3D_ResourcePack* pack);

