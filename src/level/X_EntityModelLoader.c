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

#include <ctype.h>

#include "X_EntityModelLoader.h"
#include "X_EntityModel.h"
#include "error/X_log.h"
#include "error/X_error.h"
#include "util/X_util.h"

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

static void read_texture_coords(X_EntityModelLoader* loader)
{
    X_EntityModel* model = loader->modelDest;
    
    model->totalTextureCoords = loader->header.totalVertices;
    model->textureCoords = x_malloc(sizeof(X_EntityTextureCoord) * model->totalTextureCoords);
    
    for(int i = 0; i < model->totalTextureCoords; ++i)
    {
        model->textureCoords[i].onSeam = x_file_read_le_int32(&loader->file);
        x_file_read_vec2(&loader->file, &model->textureCoords[i].coord);
    }
}

static void read_triangles(X_EntityModelLoader* loader)
{
    X_EntityModel* model = loader->modelDest;
    
    model->totalTriangles = loader->header.totalTriangles;
    model->triangles = x_malloc(sizeof(X_EntityTriangle) * model->totalTriangles);
    
    for(int i = 0; i < model->totalTriangles; ++i)
    {
        model->triangles[i].facesFront = x_file_read_le_int32(&loader->file);
        
        for(int v = 0; v < 3; ++v)
            model->triangles[i].vertexIds[v] = x_file_read_le_int32(&loader->file);
    }
}

static void read_vertex(X_File* file, X_EntityVertex* vertex)
{
    vertex->v.x = x_fp16x16_from_int(x_file_read_char(file));
    vertex->v.y = x_fp16x16_from_int(x_file_read_char(file));
    vertex->v.z = x_fp16x16_from_int(x_file_read_char(file));
    vertex->v = x_vec3_convert_quake_coord_to_x3d_coord(&vertex->v);
    
    
    vertex->normalIndex = x_file_read_char(file);
}

static void read_boundbox(X_File* file, X_EntityBoundBox* boundBox)
{
    read_vertex(file, &boundBox->min);
    read_vertex(file, &boundBox->max);
}

static void read_frame(X_EntityModelLoader* loader, X_EntityFrame* frame)
{
    read_boundbox(&loader->file, &frame->boundBox);
    x_file_read_buf(&loader->file, 16, frame->name);
    
    frame->vertices = x_malloc(sizeof(X_EntityVertex) * loader->header.totalVertices);
    
    printf("Frame name: %s\n", frame->name);
    
    for(int i = 0; i < loader->header.totalVertices; ++i)
        read_vertex(&loader->file, frame->vertices + i);
}

static void read_frame_group(X_EntityModelLoader* loader, X_EntityFrameGroup* group)
{
    _Bool partOfGroup = x_file_read_le_int32(&loader->file);
    
    group->totalFrames = (partOfGroup ? x_file_read_le_int32(&loader->file) : 1);
    group->frames = x_malloc(sizeof(X_EntityFrame) * group->totalFrames);

    if(group->totalFrames != 1)
    {
        x_system_error("Model %s has a frame group, which is not supported yet (group size = %d)", loader->modelDest->name, group->totalFrames);
    }
    
    for(int i = 0; i < group->totalFrames; ++i)
        read_frame(loader, group->frames + i);
}

static void read_frame_groups(X_EntityModelLoader* loader)
{
    X_EntityModel* model = loader->modelDest;
    
    model->totalFrameGroups = loader->header.totalFrames;
    model->frameGroups = x_malloc(sizeof(X_EntityFrameGroup) * model->totalFrameGroups);
    
    for(int i = 0; i < loader->header.totalFrames; ++i)
        read_frame_group(loader, model->frameGroups + i);
}

static _Bool split_frame_name_into_base_and_number(const char* frameName, char* baseName, char* frameNumber)
{
    const char* frameNumberStart = frameName + strlen(frameName) - 1;
    
    while(isdigit(*frameNumberStart))
        --frameNumberStart;
    
    strcpy(frameNumber, frameNumberStart + 1);
    
    while(frameName <= frameNumberStart)
        *baseName++ = *frameName++;

    *baseName = '\0';
    
    return *frameNumber != '\0';
}

static void stitch_frames_into_animations(X_EntityModel* model)
{
    char baseName[16];
    char frameNumber[16];
    
    for(int i = 1; i < model->totalFrameGroups; ++i)
    {
        X_EntityFrame* frame = model->frameGroups[i].frames + 0;
        frame->nextInSequence = NULL;
        
        if(!split_frame_name_into_base_and_number(frame->name, baseName, frameNumber))
            continue;
        
        char previousFrameName[20];
        sprintf(previousFrameName, "%s%d", baseName, atoi(frameNumber) - 1);
        
        X_EntityFrame* prevFrame = model->frameGroups[i - 1].frames + 0;
        
        if(strcmp(prevFrame->name, previousFrameName) == 0)
            prevFrame->nextInSequence = frame;
    }
}

static _Bool read_contents(X_EntityModelLoader* loader)
{
    if(!read_header(loader))
        return 0;
    
    read_skins(loader);
    read_texture_coords(loader);
    read_triangles(loader);
    read_frame_groups(loader);
    
    stitch_frames_into_animations(loader->modelDest);
    
    return 1;
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

_Bool x_entitymodelloader_load_model_from_file(X_EntityModelLoader* loader, const char* fileName, X_EntityModel* dest)
{
    char modelName[256];
    x_filepath_extract_filename(fileName, modelName);
    x_strncpy(dest->name, modelName, sizeof(dest->name) - 1);
    
    if(!x_file_open_reading(&loader->file, fileName))
        return 0;
    
    loader->modelDest = dest;
    _Bool success = read_contents(loader);
    print_header(&loader->header);
    
    x_file_close(&loader->file);
    
    return success;
}

