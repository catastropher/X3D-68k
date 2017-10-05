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

_Bool x_entitymodel_load_from_file(X_EntityModel* model, const char* fileName)
{
    X_EntityModelLoader loader;
    
    if(!x_entitymodelloader_load_model_from_file(&loader, fileName, model))
    {
        x_log_error("Failed to load model %s\n", fileName);
        return 0;
    }
    
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
}

