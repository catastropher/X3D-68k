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

