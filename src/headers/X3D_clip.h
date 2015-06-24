// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"

#pragma once

//=============================================================================
// Forward declarations
//=============================================================================

struct X3D_Frustum;
struct X3D_RenderContext;

//=============================================================================
// Structures
//=============================================================================

typedef struct X3D_FailPlane {
  int16 plane_d;
  int16 dot;
} X3D_FailPlane;

/// @todo document
// Holds which planes in a frustum a vertex is outside of
// Note: this is a variable sized data structure
typedef struct X3D_VertexClip {
  X3D_Vex3D_int16 v;

  uint16 total_fp;
  X3D_FailPlane fp[0];
} X3D_VertexClip;

typedef struct X3D_Edge {
  X3D_Vex3D_int16 v[2];
} X3D_Edge;

typedef struct X3D_ParamLine2D {
  X3D_Vex2D_int16 start;
  X3D_Vex2D_int16 normal;
  int32 d;
} X3D_ParamLine2D;

typedef struct X3D_ClipRegion {
  uint16 total_pl;
  X3D_ParamLine2D pl[0];
} X3D_ClipRegion;

typedef struct X3D_Prism2D {
  uint32 draw_edges;        ///< Bitfield of which edges should be drawn
  uint16 base_v;            ///< Number of vertices in each base
  X3D_Vex2D_int16 v[0];     ///< Vertices (variable number)
} X3D_Prism2D;

//=============================================================================
// Function declarations
//=============================================================================

void x3d_get_fail_planes(X3D_VertexClip* vc, X3D_Vex3D_int16* v, X3D_Frustum* f);
void x3d_frustum_from_rendercontext(struct X3D_Frustum* f, struct X3D_RenderContext* context);

