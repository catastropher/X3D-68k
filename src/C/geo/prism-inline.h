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
#include "X3D_prism.h"
#include "X3D_trig.h"

//=============================================================================
// x3d_prism3d_construct()
//=============================================================================

static inline void x3d_prism3d_construct_top_base(X3D_Prism3D* prism, int16 radius, int16 height) {
    ufp8x8 base_angle = 0;
    ufp8x8 angle_step = 65536L / prism->base_v;
    int16 prism_base_y = -height / 2;
    
    for(int i = 0; i < prism->base_v; ++i) {
        X3D_Vex2D circle_point;
        x3d_vex2d_make_point_on_circle(radius, x3d_uint16_upper(base_angle), &circle_point);
        prism->v[i] = x3d_vex3d_make(circle_point.x, prism_base_y, circle_point.y);
        
        base_angle += angle_step;
    }
}

static inline void x3d_prism3d_construct_bottom_base_from_top(X3D_Prism3D* prism, int16 height) {
    for(int i = 0; i < prism->base_v; ++i) {
        prism->v[i + prism->base_v].x = prism->v[i].x;
        prism->v[i + prism->base_v].y = height / 2;
        prism->v[i + prism->base_v].z = prism->v[i].z;
    }
}

static inline uint16 x3d_prism3d_calculate_radius_from_side_length(uint16 side_length, uint16 sides_in_base) {
    return ((int32)side_length << 15) / (2 * x3d_sin(ANG_180 / sides_in_base)); 
}

//=============================================================================
// x3d_prism3d_get_face()
//=============================================================================

static inline void x3d_prism3d_side_face_vertex_ids(X3D_Prism3D* prism, uint16 face, uint16* ids_dest) {
    uint16 side_id = face - 2;
    uint16 next_id = x3d_int16_increment_wrap(side_id, prism->base_v);
    
    ids_dest[0] = side_id;
    ids_dest[1] = side_id + prism->base_v;
    ids_dest[2] = next_id + prism->base_v;
    ids_dest[3] = next_id;
}

static inline void x3d_prism3d_get_face_base_a(X3D_Prism3D* prism, X3D_Polygon3D* dest) {
    for(int16 i = 0; i < prism->base_v; ++i)
        dest->v[i] = prism->v[i];
    
    dest->total_v = prism->base_v;
}

static inline void x3d_prism3d_get_face_base_b_reversed(X3D_Prism3D* prism, X3D_Polygon3D* dest) {
    for(int16 i = 0; i < prism->base_v; ++i)
        dest->v[i] = prism->v[prism->base_v + (prism->base_v - i - 1)];
    
    dest->total_v = prism->base_v;
}

static inline void x3d_prism3d_get_face_side(X3D_Prism3D* prism, uint16 face, X3D_Polygon3D* dest) {
    uint16 side_vertex_ids[4];
    x3d_prism3d_side_face_vertex_ids(prism, face, side_vertex_ids);
    
    for(int16 i = 0; i < 4; ++i)
        dest->v[i] = prism->v[side_vertex_ids[i]];
    
    dest->total_v = 4;
}

//=============================================================================
// x3d_prism3d_set_face()
//=============================================================================


static inline void x3d_prism3d_set_face_base_a(X3D_Prism3D* prism, X3D_Polygon3D* src) {
    for(int16 i = 0; i < prism->base_v; ++i)
        prism->v[i] = src->v[i];
}

static inline void x3d_prism3d_set_face_base_b_reversed(X3D_Prism3D* prism, X3D_Polygon3D* src) {
    for(int16 i = 0; i < prism->base_v; ++i)
        prism->v[prism->base_v + (prism->base_v - i - 1)] = src->v[i];
}

static inline void x3d_prism3d_set_face_side(X3D_Prism3D* prism, uint16 face, X3D_Polygon3D* src) {
    uint16 side_vertex_ids[4];
    x3d_prism3d_side_face_vertex_ids(prism, face, side_vertex_ids);
    
    for(int16 i = 0; i < 4; ++i)
         prism->v[side_vertex_ids[i]] = src->v[i];
}

//=============================================================================
// x3d_prism_get_edge_vertex_ids()
//=============================================================================

static inline _Bool x3d_prism_edge_is_part_of_base(uint16 edge_id, uint16 base_v) {
    return edge_id < base_v * 2;
}

static inline uint16 x3d_prism_find_base_starting_vertex_from_edge(uint16 edge_id, uint16 base_v) {
    return edge_id < base_v ? 0 : base_v;
}

void x3d_prism_get_edge_vertex_ids(uint16 base_v, uint16 edge_id, uint16* start_dest, uint16* end_dest) {
    if(x3d_prism_edge_is_part_of_base(edge_id, base_v)) {
        uint16 base_start_vertex = x3d_prism_find_base_starting_vertex_from_edge(edge_id, base_v);
        uint16 vertex_relative_to_base_start = edge_id - base_start_vertex;
        
        *start_dest = edge_id;
        *end_dest = x3d_int16_increment_wrap(vertex_relative_to_base_start, base_v) + base_start_vertex;
    }
    else {
        *start_dest = edge_id - base_v * 2;
        *end_dest = edge_id - base_v;
    }
}

