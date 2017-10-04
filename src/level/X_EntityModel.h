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

typedef struct X_EntityModel
{
    int totalSkins;
    X_EntitySkin* skins;
    int skinWidth;
    int skinHeight;
} X_EntityModel;

void x_entitymodel_get_skin_texture(X_EntityModel* model, int skinId, int textureId, X_Texture* dest);

