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
#include "file/X3D_resourcepack.h"
#include "file/X3D_buffer.h"

static _Bool x3d_resourcepack_has_valid_pack_header(X3D_ResourcePack* pack) {
    char pack_string[5] = { '\0' };
    x3d_file_read_string(pack->file, pack_string, 4);
    
    return strcmp(pack_string, "PACK") == 0;
}

static void x3d_resourcepack_read_file_entry(X3D_ResourcePack* pack, X3D_ResourcePackFile* dest) {
    x3d_file_read_string(pack->file, dest->name, X3D_RESOURCEPACK_MAX_FILENAME_LENGTH);
    dest->data_offset = x3d_file_read_little_endian_int32(pack->file);
    dest->size = x3d_file_read_little_endian_int32(pack->file);
    dest->loaded_data = NULL;
    dest->ref_count = 0;
}

static void x3d_resourcepack_read_file_table(X3D_ResourcePack* pack) {
    size_t file_table_offset = x3d_file_read_little_endian_int32(pack->file);
    size_t file_table_size = x3d_file_read_little_endian_int32(pack->file);
    
    pack->total_files = file_table_size / X3D_RESOURCEPACK_FILE_ENTRY_SIZE;
    pack->pack_files = malloc(sizeof(X3D_ResourcePackFile) * pack->total_files);
    
    x3d_file_seek(pack->file,   file_table_offset);
    
    for(int i = 0; i < pack->total_files; ++i)
        x3d_resourcepack_read_file_entry(pack, pack->pack_files + i);
}

_Bool x3d_resourcepack_load_from_file(X3D_ResourcePack* pack, const char* file_name) {
    pack->file = fopen(file_name, "rb");
    
    if(!pack->file) {
        x3d_log(X3D_ERROR, "Failed to open pack file '%s'", file_name);
        return X3D_FALSE;
    }
    
    if(!x3d_resourcepack_has_valid_pack_header(pack)) {
        x3d_log(X3D_ERROR, "Pack file '%s' has bad pack header", file_name);
        return X3D_FALSE;
    }
    
    x3d_resourcepack_read_file_table(pack);
    
    return X3D_TRUE;
}

static _Bool x3d_resourcepack_load_packfile_into_buffer(X3D_ResourcePack* pack, int file_id, X3D_Buffer* dest) {
    X3D_ResourcePackFile* pack_file = pack->pack_files + file_id;
    
    if(!pack_file->loaded_data) {
        if(!pack->file)
            return X3D_FALSE;
        
        pack_file->loaded_data = malloc(pack_file->size);
        x3d_file_seek(pack->file, pack_file->data_offset);
        x3d_file_read_buf(pack->file, pack_file->loaded_data, pack_file->size);
    }

    x3d_buffer_init_existing(dest, pack_file->loaded_data, pack_file->size, file_id);
    ++pack_file->ref_count;
    
    return X3D_TRUE;
}

_Bool x3d_resourcepack_open_packfile(X3D_ResourcePack* pack, const char* file_name, X3D_Buffer* dest) {
    for(int i = 0; i < pack->total_files; ++i) {
        if(strcmp(file_name, pack->pack_files[i].name) == 0) {
            return x3d_resourcepack_load_packfile_into_buffer(pack, i, dest);
        }
    }
    
    return X3D_FALSE;
}

static void x3d_resourcepack_close_packfile_given_id(X3D_ResourcePack* pack, int id, _Bool force_close) {
    X3D_ResourcePackFile* pack_file = pack->pack_files + id;
    
    --pack_file->ref_count;
    
    if(!pack_file->loaded_data)
        return;
    
    if(pack_file->ref_count == 0 || force_close) {
        free(pack_file->loaded_data);
        pack_file->loaded_data = NULL;
        pack_file->ref_count = 0;
    }
}

void x3d_resourcepack_close_packfile(X3D_ResourcePack* pack, X3D_Buffer* data_src) {
    if(data_src->id < 0 || data_src->id >= pack->total_files) {
        x3d_log(X3D_ERROR, "Attempting to close packfile with bad id");
        return;
    }
    
    x3d_resourcepack_close_packfile_given_id(pack, data_src->id, X3D_FALSE);
    x3d_buffer_init_empty(data_src);
}

void x3d_resourcepack_close_all_packfiles(X3D_ResourcePack* pack) {
    for(int i = 0; i < pack->total_files; ++i)
        x3d_resourcepack_close_packfile_given_id(pack, i, X3D_TRUE);
}

void x3d_resourcepack_cleanup(X3D_ResourcePack* pack) {
    x3d_resourcepack_close_all_packfiles(pack);
    
    free(pack->pack_files);
    pack->pack_files = NULL;
    pack->total_files = 0;
    
    fclose(pack->file);
}

static inline void x3d_resourcepack_write_pack_header(X3D_ResourcePack* pack) {
    fputs("PACK", pack->file);
    x3d_file_write_little_endian_int32(pack->file, X3D_RESOURCEPACK_HEADER_SIZE);
    x3d_file_write_little_endian_int32(pack->file, pack->total_files * X3D_RESOURCEPACK_FILE_ENTRY_SIZE);
}

static inline void x3d_resourcepack_calculate_file_offsets(X3D_ResourcePack* pack) {
    pack->pack_files[0].data_offset = X3D_RESOURCEPACK_HEADER_SIZE + pack->total_files * X3D_RESOURCEPACK_FILE_ENTRY_SIZE;
    
    for(int i = 1; i < pack->total_files; ++i)
        pack->pack_files[i].data_offset = pack->pack_files[i - 1].data_offset + pack->pack_files[i - 1].size;
}

static inline void x3d_resourcepack_write_file_header(X3D_ResourcePack* pack, X3D_ResourcePackFile* pack_file) {
    x3d_file_write_buf(pack->file, pack_file->name, X3D_RESOURCEPACK_MAX_FILENAME_LENGTH);
    x3d_file_write_little_endian_int32(pack->file, pack_file->data_offset);
    x3d_file_write_little_endian_int32(pack->file, pack_file->size);
}

static void x3d_resourcepack_write_file_table(X3D_ResourcePack* pack) {
    if(pack->total_files == 0)
        return;
    
    x3d_resourcepack_calculate_file_offsets(pack);
    
    for(int i = 0; i < pack->total_files; ++i)
        x3d_resourcepack_write_file_header(pack, pack->pack_files + i);
}

static void x3d_resourcepack_write_file_data(X3D_ResourcePack* pack) {
    for(int i = 0; i < pack->total_files; ++i)
        x3d_file_write_buf(pack->file, pack->pack_files[i].loaded_data, pack->pack_files[i].size);
}

_Bool x3d_resourcepack_save_packfiles_to_file(X3D_ResourcePackFile* pack_files, int total_files, const char* file_name) {
    X3D_ResourcePack pack;
    pack.pack_files = pack_files;
    pack.total_files = total_files;
    pack.file = fopen(file_name, "wb");
    
    if(!pack.file) {
        x3d_log(X3D_ERROR, "Could not open pack file '%s' for writing", file_name);
        return X3D_FALSE;
    }
    
    x3d_resourcepack_write_pack_header(&pack);
    x3d_resourcepack_write_file_table(&pack);
    x3d_resourcepack_write_file_data(&pack);
    
    fclose(pack.file);
    
    return X3D_TRUE;
}

void x3d_resourcepack_print_file_header(X3D_ResourcePack* pack) {
    x3d_log(X3D_INFO, "Total files in pack: %d", (int)pack->total_files);
    
    for(int i = 0; i < pack->total_files; ++i) {
        x3d_log(X3D_INFO, "\t%56s (size: %d, offset: %d", pack->pack_files[i].name, (int)pack->pack_files[i].size, (int)pack->pack_files[i].data_offset);
    }
}

