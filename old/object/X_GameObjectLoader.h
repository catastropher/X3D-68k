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
#include "engine/X_EngineContext.h"

// In this file, "edict" stands for "entity dictionary"

#define X_EDICT_MAX_NAME_LENGTH 32
#define X_EDICT_MAX_ATTRIBUTES 64

typedef struct X_EdictAttribute
{
    char name[X_EDICT_MAX_NAME_LENGTH];
    char* value;
} X_EdictAttribute;

typedef struct X_Edict
{
    X_EdictAttribute attributes[X_EDICT_MAX_ATTRIBUTES];
    int totalAttributes;
} X_Edict;

void x_gameobjectloader_load_objects(X_EngineContext* engineContext, const char* entityDictionary);

X_EdictAttribute* x_edict_get_attribute(X_Edict* edict, const char* name);
bool x_edict_has_attribute(X_Edict* edict, const char* name);
bool x_edict_get_int(X_Edict* edict, const char* name, int defaultValue, int* dest);
bool x_edict_get_vec3(X_Edict* edict, const char* name, Vec3 defaultValue, Vec3* dest);
int x_edict_get_model_id(X_Edict* edict, const char* name);
bool x_edict_get_fp16x16(X_Edict* edict, const char* name, x_fp16x16 defaultValue, x_fp16x16* dest);
bool x_edict_get_str(X_Edict* edict, const char* name, const char* defaultValue, char* dest);

