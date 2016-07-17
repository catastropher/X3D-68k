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

#include "X3D_common.h"

enum {
  X3D_LINETEXTURE_NORMAL,
  X3D_LINETEXTURE_COMPOSITE
};

typedef struct X3D_LineTextureBase {
  uint16 texture_type;
} X3D_LineTextureBase;

typedef struct X3D_LineTexture2D {
  X3D_LineTextureBase base;
  uint8 total_v;
  uint8 total_e;
  X3D_Vex2D* v;
  X3D_Pair* edges;
} X3D_LineTexture2D;

typedef struct X3D_LineTexture3D {
  X3D_LineTextureBase base;
  uint8 total_v;
  uint8 total_e;
  X3D_Vex3D* v;
  X3D_Pair* edges;
} X3D_LineTexture3D;

typedef struct X3D_LineTextureReference {
  X3D_LineTextureBase* texture;
  angle256             rot_angle;
  X3D_Vex2D            include_pos;
} X3D_LineTextureReference;

typedef struct X3D_LineTextureComposite {
  X3D_LineTextureBase base;
  uint16 total_subtextures;
  X3D_LineTextureReference* sub_textures;
} X3D_LineTextureComposite;

struct X3D_Plane;

void x3d_linetexture2d_create_dynamically_allocated_texture(X3D_LineTexture2D* tex, uint16 max_verteices, uint16 max_edges);
uint16 x3d_linetexture2d_add_vertex(X3D_LineTexture2D* tex, X3D_Vex2D v);
uint16 x3d_linetexture2d_add_edge(X3D_LineTexture2D* tex, X3D_Vex2D a, X3D_Vex2D b);
void x3d_linetexturecomposite_create_dynamically_allocated_texture(X3D_LineTextureComposite* tex, uint16 max_subtextures);
void x3d_linetexturecomposite_add_subtexture(X3D_LineTextureComposite* tex, X3D_LineTextureBase* subtex, angle256 angle, X3D_Vex2D pos);

_Bool x3d_linetexture2d_save_to_file(X3D_LineTexture2D* tex, const char* filename);
_Bool x3d_linetexture2d_load_from_file(X3D_LineTexture2D* dest, const char* filename);

void x3d_linetexture3d_create_dynamically_allocated_texture(X3D_LineTexture3D* tex, uint16 max_verteices, uint16 max_edges);

void x3d_linetexture2d_convert_to_linetexture3d(X3D_LineTexture2D* tex, X3D_LineTexture3D* dest, struct X3D_Plane* wall_plane, X3D_Vex3D* point_on_wall);
