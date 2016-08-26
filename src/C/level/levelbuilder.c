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
#include "level/X3D_level.h"
#include "X3D_polygon.h"
#include "level/X3D_levelbuilder.h"

enum X3D_LevelBuilderSelectionType {
TYPE_NONE,
TYPE_SEGMENT,
TYPE_FACE,
TYPE_VERTEX,
TYPE_POINT_ON_FACE
} X3D_LevelBuilderSelectionType;




void x3d_levelbuilder_init(X3D_LevelBuilder* builder) {
    for(uint16 i = 0; i < X3D_LEVELBUILDER_MAX_SELECTIONS; ++i)
        builder->selections[i].type = TYPE_NONE;

    builder->current_selection = 0;
}

void x3d_levelbuilder_cleanup(X3D_LevelBuilder* builder) {

}

void x3d_levelbuilder_set_selectionid(X3D_LevelBuilder* builder, uint16 selection) {
    builder->current_selection = selection;
}

void x3d_levelbuilder_select_segment(X3D_LevelBuilder* builder, X3D_LEVEL_SEG seg) {
    builder->selections[builder->current_selection].type = TYPE_SEGMENT;
    builder->selections[builder->current_selection].segment_selection.segid = seg;
}

void x3d_levelbuilder_select_segmentface(X3D_LevelBuilder* builder, X3D_SegFaceID face_id) {
    builder->selections[builder->current_selection].type = TYPE_FACE;
    builder->selections[builder->current_selection].segment_selection.segid = face_id;
}

void x3d_levelbuilder_extrude(X3D_LevelBuilder* builder, int16 extrude_amount) {
    X3D_LevelBuilderSelection* select = builder->selections + builder->current_selection;
    X3D_LevelSegment* new_seg;

//     switch(select->type) {
//         case TYPE_FACE:
//             new_seg = x3d_level_add_extruded_segment(builder->level, select->face_selection.faceid, extrude_amount);
//             x3d_levelbuilder_select_segmentface(builder->level, x3d_segfaceid_create());
//             
//     }
}


/* =========================Not yet implemented=========================

void x3d_levelbuilder_select_segmentface(X3D_LevelBuilder* builder, X3D_SegFaceID face_id);
void x3d_levelbuilder_select_vertex(X3D_LevelBuilder* builder, X3D_LEVEL_VERTEX vertex);
void x3d_levelbuilder_select_point_on_face(X3D_LevelBuilder* builder, X3D_Vex2D position_relative_to_center);

void x3d_levelbuilder_scale(X3D_LevelBuilder* builder, fp8x8 scale);
void x3d_levelbuilder_translate(X3D_LevelBuilder* builder, X3D_Vex3D translation);
void x3d_levelbuilder_delete(X3D_LevelBuilder* builder);

void x3d_levelbuilder_set_extrude_shape(X3D_LevelBuilder* builder, X3D_Polygon2D* extrude_shape);

void x3d_levelbuilder_connect(X3D_LevelBuilder* builder, uint16 selection_a, uint16 selection_b);
void x3d_levelbuilder_add_attachment(X3D_LevelBuilder* builder);

void x3d_levelbuilder_add_resource(X3D_LevelBuilder* builder);
void x3d_levelbuilder_delete_resource(X3D_LevelBuilder* builder);

void x3d_levelbuilder_set_inside_mode(X3D_LevelBuilder* builder);
void x3d_levelbuilder_set_outside_mode(X3D_LevelBuilder* builder);
*/
