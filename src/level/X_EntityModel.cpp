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

#include "X_EntityModel.h"
#include "X_EntityModelLoader.h"
#include "error/X_log.h"
#include "error/X_error.h"

#include "render/X_RenderContext.h"
#include "geo/X_Ray3.h"
#include "geo/X_Polygon3.h"

bool x_entitymodel_load_from_file(X_EntityModel* model, const char* fileName)
{
    X_EntityModelLoader loader;
    
    if(!x_entitymodelloader_load_model_from_file(&loader, fileName, model))
    {
        x_log_error("Failed to load model %s\n", fileName);
        return 0;
    }
    
    x_log("Loaded model %s", model->name);
    
    return 1;
}

void x_entitymodel_get_skin_texture(X_EntityModel* model, int skinId, int textureId, X_Texture* dest)
{
    x_assert(skinId < model->totalSkins && textureId < model->skins[skinId].totalTextures, "Bad skin/texture id");
    dest->texels = model->skins[skinId].textures[textureId].texels;
    dest->w = model->skinWidth;
    dest->h = model->skinHeight;
}

void x_entitymodel_cleanup(X_EntityModel* model)
{
    // Each skin allocates its textures together, so just free the first
    for(int i = 0; i < model->totalSkins; ++i)
    {
        x_free(model->skins[i].textures[0].texels);
        x_free(model->skins[i].textures);
    }
    
    x_free(model->skins);
    x_free(model->textureCoords);
    x_free(model->triangles);
    
    for(int i = 0; i < model->totalFrameGroups; ++i)
    {
        X_EntityFrameGroup* group = model->frameGroups + i;
        for(int frameId = 0; frameId < group->totalFrames; ++frameId)
        {
            X_EntityFrame* frame = group->frames + frameId;
            x_free(frame->vertices);
        }
        
        x_free(group->frames);
    }
    
    x_free(model->frameGroups);
}

X_EntityFrame* x_entitymodel_get_frame(X_EntityModel* model, const char* frameName)
{
    for(int i = 0; i < model->totalFrameGroups; ++i)
    {
        X_EntityFrameGroup* group = model->frameGroups + i;
        
        for(int frameId = 0; frameId < group->totalFrames; ++frameId)
        {
            if(strcmp(group->frames[frameId].name, frameName) == 0)
                return group->frames + frameId;
        }
    }
    
    return NULL;
}

X_EntityFrame* x_entitymodel_get_animation_start_frame(X_EntityModel* model, const char* animationName)
{
    char startFrameName[20];
    sprintf(startFrameName, "%s1", animationName);
    return x_entitymodel_get_frame(model, startFrameName);
}

void x_entitymodel_draw_frame_wireframe(X_EntityModel* model, X_EntityFrame* frame, X_Vec3 pos, X_Color color, X_RenderContext* renderContext)
{
    for(X_EntityTriangle* triangle = model->triangles; triangle < model->triangles + model->totalTriangles; ++triangle)
    {
        X_Vec3 v[3];
        X_EntityVertex* vertices[3];
        
        for(int i = 0; i < 3; ++i)
        {
            vertices[i] = frame->vertices + triangle->vertexIds[i];
            v[i] = x_vec3_add(&vertices[i]->v, &pos);
        }
        
        for(int i = 0; i < 3; ++i)
        {
            // FIXME: this is broken
            //X_Ray3_fp16x16 ray = x_ray3_make(x_vec3_fp16x16_to_vec3(&v[i]), x_vec3_fp16x16_to_vec3(&v[(i + 1) % 3]));
            //x_ray3_render(&ray, renderContext, color);
        }
    }
}

void x_entitymodel_render_flat_shaded(X_EntityModel* model, X_EntityFrame* frame, X_RenderContext* renderContext)
{
    for(int i = 0; i < model->totalTriangles; ++i)
    {
        X_Vec3_int v[3];
        X_Polygon3 poly = x_polygon3_make(v, 3);
        X_EntityTriangle* tri = model->triangles + i;
        X_Vec2 textureCoords[3];
        
        for(int j = 0; j < 3; ++j)
        {
            v[j] = x_vec3_to_vec3_int(&frame->vertices[tri->vertexIds[j]].v);
            
            X_EntityTextureCoord* coord = model->textureCoords + tri->vertexIds[j];
            
            textureCoords[j] = coord->coord;
            if(!tri->facesFront && coord->onSeam)
                textureCoords[j].x += model->skinWidth / 2;
        }
        
        X_Texture skin;
        x_entitymodel_get_skin_texture(model, 0, 0, &skin);
        
        x_polygon3_render_textured(&poly, renderContext, &skin, textureCoords);
    }
}


