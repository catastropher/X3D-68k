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
#include "X3D_screen.h"

enum {
  X3D_PORTAL_REMOTE = 1,
  X3D_PORTAL_DRAW_OUTLINE = 2,
  X3D_PORTAL_FILL = 3
};

typedef struct X3D_Portal {
  uint16 flags;
  uint16 total_v;
  X3D_Vex3D* v;
  
  X3D_Color outline_color;
  X3D_Color fill_color;
  
  X3D_RasterRegion* region;
  X3D_Plane* plane;
} X3D_Portal;

///////////////////////////////////////////////////////////////////////////////
/// Enables filling a portal with a solid color and sets the fill color.
///
/// @param portal - portal
/// @param color  - color to fill the portal with
///
/// @note The portal is filled before anything on the other side of the portal
///   is drawn.
///////////////////////////////////////////////////////////////////////////////
static inline void x3d_portal_set_fill(X3D_Portal* portal, X3D_Color color) {
  portal->flags |= X3D_PORTAL_FILL;
  portal->fill_color = color;
}

static inline _Bool x3d_portal_fill(X3D_Portal* portal) {
  return portal->flags & X3D_PORTAL_FILL;
}

static inline X3D_Color x3d_portal_fill_color(X3D_Portal* portal) {
  return portal->fill_color;
}

void x3d_portal_render(X3D_Portal* portal);

