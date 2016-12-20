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
#include "X3D_matrix.h"
#include "X3D_trig.h"
#include "X3D_vector.h"
#include "X3D_prism.h"
#include "X3D_polygon.h"
#include "X3D_camera.h"
#include "X3D_render.h"
#include "X3D_enginestate.h"
#include "geo/X3D_line.h"

#include "prism-inline.h"

void x3d_prism3d_construct(X3D_Prism3D* prism, uint16 sides_in_base, uint16 side_length, int16 height, X3D_Vex3D_angle256 angle) {
    uint16 prism_radius = x3d_prism3d_calculate_radius_from_side_length(side_length, sides_in_base);
    
    x3d_prism3d_set_base_v(prism, sides_in_base);
    x3d_prism3d_construct_top_base(prism, prism_radius, height);
    x3d_prism3d_construct_bottom_base_from_top(prism, height);
    x3d_prism3d_rotate_around_origin(prism, angle);
}

void x3d_prism3d_rotate_around_origin(X3D_Prism3D* prism, X3D_Vex3D_angle256 angle) {
    X3D_Mat3x3_fp0x16 mat;
    x3d_mat3x3_construct(&mat, &angle);

    for(int i = 0; i < x3d_prism3d_total_vertices(prism); ++i) {
        X3D_Vex3D_int16 rotated;
        x3d_vex3d_int16_rotate(&rotated, &prism->v[i], &mat);
        prism->v[i] = rotated;
    }
}

void x3d_prism3d_get_face(const X3D_Prism3D* prism, uint16 face_id, X3D_Polygon3D* dest) {
    if(face_id == X3D_BASE_A)
        x3d_prism3d_get_face_base_a(prism, dest);
    else if(face_id == X3D_BASE_B)
        x3d_prism3d_get_face_base_b_reversed(prism, dest);
    else
        x3d_prism3d_get_face_side(prism, face_id, dest);
}

void x3d_prism3d_set_face(X3D_Prism3D* prism, uint16 face_id, const X3D_Polygon3D* src) {
    if(face_id == X3D_BASE_A)
        x3d_prism3d_set_face_base_a(prism, src);
    else if(face_id == X3D_BASE_B)
        x3d_prism3d_set_face_base_b_reversed(prism, src);
    else
        x3d_prism3d_set_face_side(prism, face_id, src);
}

void x3d_prism3d_get_edge(const X3D_Prism3D* prism, uint16 edge, X3D_Ray3D* dest) {
    uint16 start, end;
    x3d_prism_get_edge_vertex_ids(prism->base_v, edge, &start, &end);
    *dest = x3d_ray3d_make(prism->v[start], prism->v[end]);
}

void x3d_prism3d_center(const X3D_Prism3D* prism, X3D_Vex3D* dest) {
    X3D_Vex3D_int32 sum = x3d_vex3d_int32_origin();
    
    for(int16 i = 0; i < prism->base_v * 2; ++i) {
        sum = x3d_vex3d_int32_add_vex3d(&sum, prism->v + i);
    }
    
    *dest = x3d_vex3d_int32_div_by_int16_as_vex3d(&sum, prism->base_v * 2);
}

void x3d_prism3d_translate(X3D_Prism3D* prism, X3D_Vex3D* translation) {
    for(int16 i = 0; i < prism->base_v * 2; ++i)
        prism->v[i] = x3d_vex3d_add(prism->v + i, translation);
}

void x3d_prism3d_set_center(X3D_Prism3D* prism, X3D_Vex3D* new_center) {
    X3D_Vex3D center;
    x3d_prism3d_center(prism, &center);
    
    X3D_Vex3D translation = x3d_vex3d_sub(new_center, &center);
    x3d_prism3d_translate(prism, &translation);
}

/// @TODO This should be deleted!
X3D_Prism3D* x3d_prism3d_construct_temp(uint16 steps, uint16 r, int16 h) {
    static struct {
        X3D_Prism3D prism;
        X3D_Vex3D v[20];
    } temp_prism3d;
    
    x3d_assert(steps <= 10);
    
    temp_prism3d.prism.v = temp_prism3d.v;
    x3d_prism3d_construct(&temp_prism3d.prism, steps, r, h, (X3D_Vex3D_angle256) { 0, 0, 0 });
    
    return &temp_prism3d.prism;
}

uint16 x3d_prism_face_edge_indexes(uint16 base_v, uint16 face, uint16* dest) {
    if(face <= X3D_BASE_B) {
        // Prism base
        uint16 start = (face == X3D_BASE_A ? 0 : base_v);
        uint16 i;
        
        for(i = 0; i < base_v; ++i)
            dest[i] = start + i;
        
        return base_v;
    }
    else {
        // Quad face
        uint16 f = face - 2;
        dest[0] = f;
        dest[1] = x3d_int16_add_wrap(f, 1, base_v) + base_v * 2;
        dest[2] = f + base_v;
        dest[3] = f + base_v * 2;
        
        return 4;
    }
}

