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
#include "math/X_fix.h"

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

typedef struct X_EntityModel
{
    int totalSkins;
    X_EntitySkin* skins;
    int skinWidth;
    int skinHeight;
    
    int totalTextureCoords;
    X_EntityTextureCoord* textureCoords;
    
    int totalTriangles;
    X_EntityTriangle* triangles;
} X_EntityModel;

_Bool x_entitymodel_load_from_file(struct X_EntityModel* model, const char* fileName);
void x_entitymodel_get_skin_texture(X_EntityModel* model, int skinId, int textureId, X_Texture* dest);
void x_entitymodel_cleanup(X_EntityModel* model);

