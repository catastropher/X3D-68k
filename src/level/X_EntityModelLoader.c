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

#include "X_EntityModelLoader.h"
#include "X_EntityModel.h"
#include "error/X_log.h"

static _Bool read_header(X_EntityModelLoader* loader)
{
    X_EntityModelHeader* header = &loader->header;
    X_File* file = &loader->file;
    
    header->id = x_file_read_le_int32(file);
    
    unsigned int FILE_MAGIC_NUMBER = 'I' + ('D' << 8) + ('P' << 16) + ('O' << 24);
    if(header->id != FILE_MAGIC_NUMBER)
    {
        x_log_error("Bad model magic number (expected %u): %u\n", FILE_MAGIC_NUMBER, header->id);
        return 0;
    }
    
    const int MODEL_VERSION = 6;
    header->version = x_file_read_le_int32(file);
    
    if(header->version != MODEL_VERSION)
    {
        x_log_error("Bad model version (expected %d): %d\n", MODEL_VERSION, header->version);
        return 0;
    }
    
    x_file_read_vec3_float_as_fp16x16(file, &header->scale);
    
    // TODO convert coordinate system
    x_file_read_vec3_float_as_fp16x16(file, &header->origin);
    
    header->radius = x_file_read_le_float32_as_fp16x16(file);
    x_file_read_vec3_float_as_fp16x16(file, &header->offsets);
    
    header->totalSkins = x_file_read_le_int32(file);
    header->skinWidth = x_file_read_le_int32(file);
    header->skinHeight = x_file_read_le_int32(file);
    header->totalVertices = x_file_read_le_int32(file);
    header->totalTriangles = x_file_read_le_int32(file);
    header->totalFrames = x_file_read_le_int32(file);
    header->syncType = x_file_read_le_int32(file);
    header->flags = x_file_read_le_int32(file);
    header->averageTriangleSize = x_file_read_le_float32_as_fp16x16(file);
    
    return 1;
}

static void read_skin_texture_duration_times(X_EntityModelLoader* loader, X_EntitySkin* skin)
{
    X_File* file = &loader->file;
    
    if(skin->totalTextures == 1)
        return;
        
    for(int i = 0; i < skin->totalTextures; ++i)
        skin->textures[i].displayDuration = x_file_read_le_float32_as_fp16x16(file);
}

static void read_skin_texture_texels(X_EntityModelLoader* loader, X_EntitySkin* skin)
{
    X_EntityModel* model = loader->modelDest;
    X_File* file = &loader->file;
    
    int skinSize = model->skinWidth * model->skinHeight;
    X_Color* textureTexels = x_malloc(skinSize * skin->totalTextures);
    
    for(int i = 0; i < skin->totalTextures; ++i)
    {
        skin->textures[i].texels = textureTexels + skinSize * i;
        x_file_read_buf(file, skinSize, skin->textures[i].texels);
    }
}

static void read_skin(X_EntityModelLoader* loader, X_EntitySkin* skin)
{
    X_EntityModel* model = loader->modelDest;
    X_File* file = &loader->file;
    
    int totalGroups = x_file_read_le_int32(file);
    
    if(totalGroups == 0)
        skin->totalTextures = 1;
    else
        skin->totalTextures = x_file_read_le_int32(file);
    
    skin->textures = x_malloc(sizeof(X_EntitySkinTexture) * skin->totalTextures);
    read_skin_texture_duration_times(loader, skin);
    read_skin_texture_texels(loader, skin);
}

static void read_skins(X_EntityModelLoader* loader)
{
    X_EntityModel* model = loader->modelDest;
    model->skins = x_malloc(sizeof(X_EntitySkin) * loader->header.totalSkins);
    model->totalSkins = loader->header.totalSkins;
    model->skinWidth = loader->header.skinWidth;
    model->skinHeight = loader->header.skinHeight;
    
    for(int i = 0; i < loader->header.totalSkins; ++i)
        read_skin(loader, model->skins + i);
}

static void print_header(X_EntityModelHeader* header)
{
    printf("Id: %X\n", header->id);
    printf("Id: %d\n", header->version);
    printf("Skin W: %d\n", header->skinWidth);
    printf("Skin H: %d\n", header->skinHeight);
    printf("Total Vertices: %d\n", header->totalVertices);
    printf("Total Triangles: %d\n", header->totalTriangles);
    printf("Total Frames: %d\n", header->totalFrames);
    printf("Total skins: %d\n", header->totalSkins);
}

static _Bool read_contents(X_EntityModelLoader* loader)
{
    if(!read_header(loader))
        return 0;
    
    read_skins(loader);
    return 1;
}

_Bool x_entitymodelloader_load_model_from_file(X_EntityModelLoader* loader, const char* fileName)
{
    if(!x_file_open_reading(&loader->file, fileName))
        return 0;
    
    _Bool success = read_contents(loader);
    print_header(&loader->header);
    
    x_file_close(&loader->file);
    return success;
}


_Bool x_entitymodel_load_from_file(X_EntityModel* model, const char* fileName)
{
    X_EntityModelLoader loader;
    loader.modelDest = model;
    
    if(!x_entitymodelloader_load_model_from_file(&loader, fileName))
    {
        x_log_error("Failed to load model %s\n", fileName);
        return 0;
    }
    
    return 1;
}

void x_entitymodel_get_skin_texture(X_EntityModel* model, int skinId, int textureId, X_Texture* dest)
{
    dest->texels = model->skins[skinId].textures[textureId].texels;
    dest->w = model->skinWidth;
    dest->h = model->skinHeight;
}

