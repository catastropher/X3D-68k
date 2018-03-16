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

#include "geo/X_Vec3.h"
#include "system/X_File.h"

typedef struct X_EntityModelHeader
{
    int id;
    int version;
    Vec3 scale;
    Vec3 origin;
    x_fp16x16 radius;
    Vec3 offsets;
    int totalSkins;
    int skinWidth;
    int skinHeight;
    int totalVertices;
    int totalTriangles;
    int totalFrames;
    int syncType;       // ???
    int flags;
    x_fp16x16 averageTriangleSize;
} X_EntityModelHeader;

struct X_EntityModel;

typedef struct X_EntityModelLoader
{
    X_File file;
    X_EntityModelHeader header;
    struct X_EntityModel* modelDest;
} X_EntityModelLoader;

bool x_entitymodelloader_load_model_from_file(X_EntityModelLoader* loader, const char* fileName, struct X_EntityModel* dest);

