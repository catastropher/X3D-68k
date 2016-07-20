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

#include "X3D_common.h"
#include "level/X3D_level_linetexture.h"
#include "X3D_plane.h"
#include "X3D_polygon.h"

void x3d_linetexture2d_create_dynamically_allocated_texture(X3D_LineTexture2D* tex, uint16 max_verteices, uint16 max_edges) {
  tex->v = malloc(sizeof(X3D_Vex2D) * max_verteices);
  tex->edges = malloc(sizeof(X3D_Pair) * max_edges);
  
  tex->total_v = 0;
  tex->total_e = 0;
  
  tex->base.texture_type = X3D_LINETEXTURE_NORMAL;
}

// Adds a new vertex to the texture. Returns the ID of the vertex.
uint16 x3d_linetexture2d_add_vertex(X3D_LineTexture2D* tex, X3D_Vex2D v) {
  uint16 i;
  for(i = 0; i < tex->total_v; ++i) {
    if(x3d_vex2d_equal(tex->v + i, &v)) {
      return i;
    }
  }
  
  tex->v[tex->total_v] = v;
  return tex->total_v++;
}

uint16 x3d_linetexture2d_add_edge(X3D_LineTexture2D* tex, X3D_Vex2D a, X3D_Vex2D b) {
  tex->edges[tex->total_e] = (X3D_Pair) {
    {
      x3d_linetexture2d_add_vertex(tex, a),
      x3d_linetexture2d_add_vertex(tex, b),
    }
  };
  
  return tex->total_e++;
}

void x3d_linetexturecomposite_create_dynamically_allocated_texture(X3D_LineTextureComposite* tex, uint16 max_subtextures) {
  tex->total_subtextures = 0;
  tex->sub_textures = malloc(sizeof(X3D_LineTextureReference) * max_subtextures);
  tex->base.texture_type = X3D_LINETEXTURE_COMPOSITE;
}

void x3d_linetexturecomposite_add_subtexture(X3D_LineTextureComposite* tex, X3D_LineTextureBase* subtex, angle256 angle, X3D_Vex2D pos) {
  tex->sub_textures[tex->total_subtextures++] = (X3D_LineTextureReference) {
    .texture = subtex,
    .rot_angle = angle,
    .include_pos = pos
  };
}

_Bool x3d_linetexture2d_save_to_file(X3D_LineTexture2D* tex, const char* filename) {
  FILE* file = fopen(filename, "wb");
  
  if(!file)
    return X3D_FALSE;
  
  fprintf(file, "XTEX 1\n");
  fprintf(file, "%d %d\n", tex->total_v, tex->total_e);
  
  uint16 i;
  for(i = 0; i < tex->total_v; ++i) {
    fprintf(file, "%d %d\n", tex->v[i].x, tex->v[i].y);
  }
  
  for(i = 0; i < tex->total_e; ++i) {
    fprintf(file, "%d %d\n", tex->edges[i].val[0], tex->edges[i].val[1]);
  }
  
  fclose(file);
  return X3D_TRUE;
}

_Bool x3d_linetexture2d_load_from_file(X3D_LineTexture2D* dest, const char* filename) {
  FILE* file = fopen(filename, "rb");
  
  if(!file)
    return X3D_FALSE;
  
  char file_magic_number[5] = { 0 };
  int version;
  
  fscanf(file, "%4s %d", file_magic_number, &version);
  
  if(strcmp("XTEX", file_magic_number) != 0) {
    x3d_log(X3D_ERROR, "Bad texture magic number for file '%s'", filename);
    return X3D_FALSE;
  }
  
  if(version != 1) {
    x3d_log(X3D_ERROR, "Unknown line texture version number %d for file '%s'", version, filename);
    return X3D_FALSE;
  }
  
  int total_v;
  int total_e;
  
  fscanf(file, "%d %d", &total_v, &total_e);
  x3d_linetexture2d_create_dynamically_allocated_texture(dest, total_v, total_e);
  
  int i;
  for(i = 0; i < total_v; ++i) {
    int x, y;
    fscanf(file, "%d %d", &x, &y);
    
    dest->v[i].x = x;
    dest->v[i].y = y;
  }
  
  for(i = 0; i < total_e; ++i) {
    int start_v, end_v;
    fscanf(file, "%d %d", &start_v, &end_v);
    
    dest->edges[i].val[0] = start_v;
    dest->edges[i].val[1] = end_v;
  }
  
#if 1
  X3D_Vex2D center = { 0, 0 };
  for(i = 0; i < total_v; ++i) {
    center.x += dest->v[i].x;
    center.y += dest->v[i].y;
  }
  
  center.x /= total_v;
  center.y /= total_v;
  
  for(i = 0; i < total_v; ++i) {
    dest->v[i].x -= center.x;
    dest->v[i].y -= center.y;
    
    dest->v[i].x /= 3;
    dest->v[i].y /= 3;
  }
#endif
  
  dest->total_v = total_v;
  dest->total_e = total_e;
  
  fclose(file);
  
  return X3D_TRUE;
}

void x3d_linetexture2d_convert_to_linetexture3d(X3D_LineTexture2D* tex, X3D_LineTexture3D* dest, X3D_Plane* wall_plane, X3D_Vex3D* point_on_wall) {
  X3D_Polygon2D poly2d = { .v = tex->v, .total_v = tex->total_v };
  X3D_Polygon3D poly3d = { .v = dest->v };
  X3D_Mat3x3 orientation;
  
  x3d_polygon2d_to_polygon3d(&poly2d, &poly3d, wall_plane, point_on_wall, point_on_wall, &orientation);
  
  uint16 i;
  for(i = 0; i < tex->total_v; ++i) {
    poly3d.v[i] = x3d_vex3d_add(poly3d.v + i, point_on_wall);
  }
  
  dest->edges = tex->edges;
  dest->total_e = tex->total_e;
  dest->total_v = tex->total_v;
}

void x3d_linetexture3d_create_dynamically_allocated_texture(X3D_LineTexture3D* tex, uint16 max_verteices, uint16 max_edges) {
  tex->v = malloc(sizeof(X3D_Vex3D) * max_verteices);
  tex->edges = malloc(sizeof(X3D_Pair) * max_edges);
  
  tex->total_v = 0;
  tex->total_e = 0;
  
  tex->base.texture_type = X3D_LINETEXTURE_NORMAL;
}

_Bool x3d_linetexture2d_vertex_in_range(X3D_Vex2D v, X3D_Vex2D top_left, X3D_Vex2D bottom_right) {
  return v.x >= top_left.x && v.x <= bottom_right.x &&
    v.y <= top_left.y && v.y >= bottom_right.y;
}
/*
_Bool x3d_linetexture2d_edge_in_range(X3D_LineTexture2D* src, X3D_Vex2D top_left, X3D_Vex2D bottom_right, uint16 edge) {
  return x3d_linetexture2d_vertex_in_range(src->v[src->edges[edge].val[0]]) &&
    x3d_linetexture2d_vertex_in_range(src->v[src->edges[edge].val[1]]);
}  
}                                                         used                                                                   
  uint16 total = 0;
  _Bool vertex_used[src->total_v];
  
  uint16 i;
  for(i = 0; i < src->total_v; ++i)
    vertex_used[i  = X3D_FALSE;
  
  *total_v = 0;
  
  for(i = 0; i < src->total_e; ++i) {
    if(x3d_linetexture2d_edge_in_range(src, top_left, bottom_right, i)) {
      ++total;
      
      if(!vertex_used[src->edges[i).val[0 ] {
        vertex_used[src->edges[i].val[0]] = X3D_TRUE;
        (*total_v)++;
      }
      
      if(!vertex_used[src->edges[i].val[1]]) {
        vertex_used[src->edges[i].val[1]] = X3D_TRUE;
        (*total_v)++;
      }
    }
  }    
  }
uint16 x3d_linetexture2d_create_from_edges_in_range_count_edges(X3D_LineTexture2D* src, X3D_Vex2D top_left, X3D_Vex2D bottom_right) {
  
}  
}

void x3d_linetexture2d_create_from_edges_in_range(X3D_LineTexture2D* src, X3D_Vex2D top_left, X3D_Vex2D bottom_right, X3D_LineTexture2D* dest) {
  
}  
}*/
