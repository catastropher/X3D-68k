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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "X_GameObjectLoader.h"
#include "error/X_error.h"
#include "X_PlatformObject.h"

X_EdictAttribute* x_edict_get_attribute(X_Edict* edict, const char* name)
{
    for(int i = 0; i < edict->totalAttributes; ++i)
    {
        if(strcmp(edict->attributes[i].name, name) == 0)
            return edict->attributes + i;
    }
    
    return NULL;
}

_Bool x_edict_has_attribute(X_Edict* edict, const char* name)
{
    return x_edict_get_attribute(edict, name) != NULL;
}

_Bool x_edict_get_int(X_Edict* edict, const char* name, int defaultValue, int* dest)
{
    X_EdictAttribute* att = x_edict_get_attribute(edict, name);
    if(!att)
    {
        *dest = defaultValue;
        return 0;
    }
    
    *dest = atoi(att->value);
    return 1;
}

_Bool x_edict_get_vec3(X_Edict* edict, const char* name, X_Vec3 defaultValue, X_Vec3* dest)
{
    X_EdictAttribute* att = x_edict_get_attribute(edict, name);
    if(!att)
    {
        *dest = defaultValue;
        return 0;
    }
    
    float x, y, z;
    sscanf(att->value, "%f %f %f", &x, &y, &z);
    
    dest->x = x_fp16x16_from_float(x);
    dest->y = x_fp16x16_from_float(y);
    dest->z = x_fp16x16_from_float(z);
    
    *dest = x_vec3_convert_quake_coord_to_x3d_coord(dest);
    return 1;
}

int x_edict_get_model_id(X_Edict* edict, const char* name)
{
    X_EdictAttribute* att = x_edict_get_attribute(edict, name);
    if(!att)
        x_system_error("Entity %s missing model id", x_edict_get_attribute(edict, "classname")->name);
    
    // Not sure why, but model starts with '*'
    return atoi(att->value + 1);
}

_Bool x_edict_get_fp16x16(X_Edict* edict, const char* name, x_fp16x16 defaultValue, x_fp16x16* dest)
{
    X_EdictAttribute* att = x_edict_get_attribute(edict, name);
    if(!att)
    {
        *dest = defaultValue;
        return 0;
    }
    
    *dest = x_fp16x16_from_float(atof(att->value));
    return 1;
}

static const char* load_next_edict(const char* nextEntry, char* valueData, X_Edict* dest)
{
    if(*nextEntry != '{')
        return NULL;
        
    nextEntry += 2;         // Skip quote and newline
    
    dest->totalAttributes = 0;
    
    while(*nextEntry && *nextEntry != '}')
    {
        X_EdictAttribute* att = dest->attributes + dest->totalAttributes++;
        char* name = att->name;
        
        ++nextEntry;        // Skip quote
        
        do
        {
            *name++ = *nextEntry++;
        } while(*nextEntry != '"');
        
        *name = '\0';
        
        nextEntry += 3;     // Skip [" "]
        
        att->value = valueData;
        
        do
        {
            *valueData++ = *nextEntry++;
        } while(*nextEntry != '"');
        
        nextEntry += 2;     // Skip quote and newline
        *valueData++ = '\0';
    }
    
    return nextEntry + 2;   // Skip '}' and newline
}

void x_gameobjectloader_load_objects(X_EngineContext* engineContext, const char* entityDictionary)
{
    const char* nextEntry = entityDictionary;
    char valueData[4096];
    X_Edict edict;
    
    while((nextEntry = load_next_edict(nextEntry, valueData, &edict)) != NULL)
    {
        X_EdictAttribute* att = x_edict_get_attribute(&edict, "classname");
        if(att && strcmp(att->value, "func_plat") == 0)
        {
            printf("Loaded object of type '%s'\n", att->value);
            for(int i = 0; i < edict.totalAttributes; ++i)
            {
                printf("\t%s -> %s\n", edict.attributes[i].name, edict.attributes[i].value);
            }
            
            x_objectfactory_create_object_from_edict(&engineContext->gameObjectFactory, &edict);
        }
    }
}

