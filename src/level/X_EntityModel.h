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

#include "render/X_Texture.h"
#include "geo/X_Vec3.h"

typedef struct X_EntitySkinTexture
{
    X_Color* texels;
    x_fp16x16 displayDuration;
} X_EntitySkinTexture;

typedef struct X_EntitySkin
{
    int totalTextures;
    X_EntitySkinTexture* textures;
} X_EntitySkin;

typedef struct X_EntityTextureCoord
{
    X_Vec2 coord;
    _Bool onSeam;
} X_EntityTextureCoord;

typedef struct X_EntityTriangle
{
    int vertexIds[3];
    _Bool facesFront;
} X_EntityTriangle;

typedef struct X_EntityVertex
{
    X_Vec3_fp16x16 v;
    int normalIndex;
} X_EntityVertex;

typedef struct X_EntityBoundBox
{
    X_EntityVertex min;
    X_EntityVertex max;
} X_EntityBoundBox;

typedef struct X_EntityFrame
{
    X_EntityBoundBox boundBox;
    char name[16];
    X_EntityVertex* vertices;
    
    struct X_EntityFrame* nextInSequence;
} X_EntityFrame;

typedef struct X_EntityFrameGroup
{
    int totalFrames;
    X_EntityFrame* frames;
    X_EntityBoundBox boundBox;
    x_fp16x16 displayDuration;
} X_EntityFrameGroup;

typedef struct X_EntityModel
{
    char name[16];
    int totalSkins;
    X_EntitySkin* skins;
    int skinWidth;
    int skinHeight;
    
    int totalTextureCoords;
    X_EntityTextureCoord* textureCoords;
    
    int totalTriangles;
    X_EntityTriangle* triangles;
    
    int totalFrameGroups;
    X_EntityFrameGroup* frameGroups;
} X_EntityModel;

_Bool x_entitymodel_load_from_file(struct X_EntityModel* model, const char* fileName);
void x_entitymodel_get_skin_texture(X_EntityModel* model, int skinId, int textureId, X_Texture* dest);
void x_entitymodel_cleanup(X_EntityModel* model);

X_EntityFrame* x_entitymodel_get_frame(X_EntityModel* model, const char* frameName);
X_EntityFrame* x_entitymodel_get_animation_start_frame(X_EntityModel* model, const char* animationName);

struct X_RenderContext;

void x_entitymodel_draw_frame_wireframe(X_EntityModel* model, X_EntityFrame* frame, X_Vec3_fp16x16 pos, X_Color color, struct X_RenderContext* renderContext);
void x_entitymodel_render_flat_shaded(X_EntityModel* model, X_EntityFrame* frame, struct X_RenderContext* renderContext);

