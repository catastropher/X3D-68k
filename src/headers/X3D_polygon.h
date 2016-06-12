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
#include "X3D_vector.h"
#include "X3D_plane.h"
#include "X3D_assert.h"
#include "X3D_matrix.h"
#include "X3D_screen.h"

#pragma once

///<  Initializes a 3D polygon and allocates space for the given number of
///   vertices on the stack.
#define X3D_POLYGON3D_ALLOCA(_poly, _total_v) { _poly->v = alloca(sizeof(X3D_Vex3D) * total_v); _poly->total_v = _total_v; }

enum {
  X3D_POLYGON_COLOR     = 1,
  X3D_POLYGON_TEXTURE   = 2,
  X3D_POLYGON_GOURAUD   = 16,
  X3D_POLYGON_DITHER    = 32
};

typedef struct X3D_PolygonAttributes {
  uint16 flags;
  
  union {
    struct {
      uint16* uu;
      uint16* vv;
      uint16 texture_id;
    } texture;
    
    uint16 color;
  };
} X3D_PolygonAttributes;

///////////////////////////////////////////////////////////////////////////////
/// A 3D polygon with a variable number of points.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Polygon3D {
  uint16 total_v;     ///< Number of vertices
  X3D_Vex3D* v;       ///< The vertices
} X3D_Polygon3D;

///////////////////////////////////////////////////////////////////////////////
/// A 2D polygon with a variable number of points.
///////////////////////////////////////////////////////////////////////////////
typedef struct X3D_Polygon2D {
  uint16 total_v;     ///< Number of vertices
  X3D_Vex2D* v;       ///< The vertices
} X3D_Polygon2D;

struct X3D_CameraObject;
struct X3D_RasterRegion;

// 3D polygon functions
void x3d_polygon3d_print(X3D_Polygon3D* p);
void x3d_polygon3d_translate_normal(X3D_Polygon3D* poly, X3D_Normal3D* dir, int16 dist);
void x3d_polygon3d_translate(X3D_Polygon3D* poly, X3D_Vex3D shift);
void x3d_polygon3d_reverse(X3D_Polygon3D* poly);
void x3d_polygon3d_center(X3D_Polygon3D* poly, X3D_Vex3D* dest);
void x3d_polygon3d_scale(X3D_Polygon3D* poly, fp8x8 scale);
void x3d_polygon3d_rotate(X3D_Polygon3D* poly, X3D_Vex3D_angle256 angle, X3D_Vex3D center);
void x3d_polygon3d_copy(X3D_Polygon3D* src, X3D_Polygon3D* dest);
void x3d_polygon3d_render(X3D_Polygon3D* poly, X3D_PolygonAttributes* att, struct X3D_CameraObject* cam, struct X3D_RasterRegion* parent);
_Bool x3d_polygon3d_clip_to_near_plane(X3D_Polygon3D* poly, X3D_Polygon3D* dest, int16 near_z, uint16* ua, uint16* va, uint16* new_ua, uint16* new_va);
_Bool x3d_polygon3d_clip_to_plane(X3D_Polygon3D* poly, X3D_Polygon3D* dest, X3D_Plane* plane, uint16* ua, uint16* va, uint16* new_ua, uint16* new_va, uint16* clip);

// 2D polygon functions
void x3d_polygon2d_construct(X3D_Polygon2D* poly, uint16 steps, int16 r, angle256 ang);
void x3d_polygon2d_add_point(X3D_Polygon2D* poly, int16 x, int16 y);
void x3d_polygon2d_remove_duplicate_points(X3D_Polygon2D* poly);
void x3d_polygon2d_to_polygon3d(X3D_Polygon2D* poly, X3D_Polygon3D* dest, X3D_Plane* plane, X3D_Vex3D* top_left, X3D_Vex3D* bottom_right, X3D_Mat3x3* mat);

///////////////////////////////////////////////////////////////////////////////
/// Calculates the plane equation of a 3D polygon.
///
/// @param poly     - poly
/// @param plane    - plane equation dest
///
/// @return Nothing.
/// @note   The polygon must have at least 3 points.
///////////////////////////////////////////////////////////////////////////////
static inline void x3d_polygon3d_calculate_plane(X3D_Polygon3D* poly, X3D_Plane* plane) {
  x3d_assert(poly->total_v >= 3);
  x3d_plane_construct(plane, poly->v, poly->v + 1, poly->v + 2);
}

