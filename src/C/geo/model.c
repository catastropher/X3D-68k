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

#include "geo/X3D_model.h"
#include "memory/X3D_alloc.h"
#include "system/X3D_file.h"

static inline _Bool x3d_model_file_has_valid_header(FILE* file) {
    char magic_string[5] = { 0 };
    fscanf(file, "%4s", magic_string);
    
    return strcmp(magic_string, "XMOD") == 0;
}

static inline void x3d_model_load_vertices(X3D_Model* dest, FILE* file) {
    dest->total_v = x3d_file_read_int32(file);
    dest->v = malloc(sizeof(X3D_Vex3D) * dest->total_v);
    
    for(int16 i = 0; i < dest->total_v; ++i) {
        dest->v[i] = x3d_file_read_vex3d(file);
    }
}

static inline void x3d_model_load_triangles(X3D_Model* dest, FILE* file) {
    dest->total_triangles = x3d_file_read_int32(file);
    dest->triangle_vertices = malloc(sizeof(uint16) * 3 * dest->total_triangles);
    
    for(int16 i = 0; i < dest->total_triangles; ++i) {
        X3D_Vex3D vertices = x3d_file_read_vex3d(file);
        
        dest->triangle_vertices[i * 3 + 0] = vertices.x;
        dest->triangle_vertices[i * 3 + 1] = vertices.y;
        dest->triangle_vertices[i * 3 + 2] = vertices.z;
    }
}

_Bool x3d_model_load_from_file(X3D_Model* dest, const char* file_name) {
    FILE* file = fopen(file_name, "rb");
    
    if(!file) {
        x3d_log(X3D_ERROR, "Cannot open model file: %s", file_name);
        return X3D_FALSE;
    }
    
    if(!x3d_model_file_has_valid_header(file)) {
        x3d_log(X3D_ERROR, "Model %s has bad header", file_name);
        return X3D_FALSE;
    }
    
    x3d_model_load_vertices(dest, file);
    x3d_model_load_triangles(dest, file);
    
    fclose(file);
    
    return X3D_TRUE;
}

