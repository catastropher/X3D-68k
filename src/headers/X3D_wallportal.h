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

#define MAX_PORTAL_POINTS 10

typedef struct X3D_WallPortal {
  uint16 id;
  X3D_SegFaceID face;
  X3D_Vex3D center;
  uint16 portal_id;
  X3D_Color color;
  X3D_Mat3x3 mat;
  X3D_Mat3x3 transform;
  _Bool fill;
  
  struct {
    X3D_Polygon3D portal_poly;
    X3D_Vex3D v[MAX_PORTAL_POINTS];
  };
} X3D_WallPortal;

void x3d_wallportals_init(void);
void x3d_wallportal_construct(uint16 wall_portal, X3D_SegFaceID face, X3D_Vex3D c, uint16 portal_id, X3D_Polygon2D* poly, X3D_Color color);
uint16 x3d_wallportal_add(X3D_SegFaceID face, X3D_Vex3D c, uint16 portal_id, X3D_Polygon2D* poly, X3D_Color color);
uint16 x3d_wall_get_wallportals(X3D_SegFaceID face, uint16* dest);
void x3d_wallportal_connect(uint16 portal_from, uint16 portal_to);
X3D_WallPortal* x3d_wallportal_get(uint16 portal_id);
void x3d_wallportal_update(uint16 id);
void x3d_wallportal_render(uint16 wall_portal_id, X3D_CameraObject* cam, X3D_RasterRegion* region);
void x3d_wallportal_transform_point(X3D_WallPortal* portal, X3D_Vex3D* v, X3D_Vex3D* dest);

