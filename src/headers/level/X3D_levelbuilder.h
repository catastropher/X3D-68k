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
#include "level/X3D_level_face.h"

#define X3D_LEVELBUILDER_MAX_SELECTIONS 2

typedef struct X3D_LevelBuilderSelection {
  int16 type;
  
  union {
    struct {
      X3D_LEVEL_SEG segid;
    } segment_selection;
    
    struct {
      X3D_SegFaceID faceid;
    } face_selection;
  };
  
} X3D_LevelBuilderSelection;

typedef struct X3D_LevelBuilder {
  X3D_LevelBuilderSelection selections[X3D_LEVELBUILDER_MAX_SELECTIONS];
  int16 current_selection;
  X3D_Level* level;
} X3D_LevelBuilder;


//=================================Functions=================================


void x3d_levelbuilder_init(X3D_LevelBuilder* builder);
void x3d_levelbuilder_cleanup(X3D_LevelBuilder* builder);

void x3d_levelbuilder_set_selectionid(X3D_LevelBuilder* builder, uint16 selection);

//void x3d_levelbuilder_add_segment(X3D_LevelBuilder* builder, X3D_Prism3D);
void x3d_levelbuilder_select_segmentface(X3D_LevelBuilder* builder, X3D_SegFaceID face_id);
void x3d_levelbuilder_select_segment(X3D_LevelBuilder* builder, X3D_LEVEL_SEG seg);
void x3d_levelbuilder_select_vertex(X3D_LevelBuilder* builder, X3D_LEVEL_VERTEX vertex);
void x3d_levelbuilder_select_point_on_face(X3D_LevelBuilder* builder, X3D_Vex2D position_relative_to_center);

void x3d_levelbuilder_scale(X3D_LevelBuilder* builder, fp8x8 scale);
void x3d_levelbuilder_translate(X3D_LevelBuilder* builder, X3D_Vex3D translation);
void x3d_levelbuilder_delete(X3D_LevelBuilder* builder);
void x3d_levelbuilder_extrude(X3D_LevelBuilder* builder, int16 extrude_amount);

void x3d_levelbuilder_set_extrude_shape(X3D_LevelBuilder* builder, X3D_Polygon2D* extrude_shape);

void x3d_levelbuilder_connect(X3D_LevelBuilder* builder, uint16 selection_a, uint16 selection_b);
void x3d_levelbuilder_add_attachment(X3D_LevelBuilder* builder);

void x3d_levelbuilder_add_resource(X3D_LevelBuilder* builder);
void x3d_levelbuilder_delete_resource(X3D_LevelBuilder* builder);

void x3d_levelbuilder_set_inside_mode(X3D_LevelBuilder* builder);
void x3d_levelbuilder_set_outside_mode(X3D_LevelBuilder* builder);
