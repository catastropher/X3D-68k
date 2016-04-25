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
#include "X3D_segment.h"

typedef struct X3D_SegmentBuilder {
  
} X3D_SegmentBuilder;

X3D_INTERNAL X3D_Segment* x3d_segmentbuilder_add_uncompressed_segment(X3D_Prism3D* prism);
uint16 x3d_segmentbuilder_add_extruded_segment(X3D_SegFaceID id, int16 dist);
uint16 x3d_segmentbuilder_add_connecting_segment(X3D_SegFaceID a, X3D_SegFaceID b);

