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

typedef struct X3D_LevelArray {
  uint16 size;
  uint16 count;
  void*  data;
} X3D_LevelArray;

typedef struct X3D_Level {
  X3D_LevelArray segs;
  X3D_LevelArray seg_v_list;
  X3D_LevelArray v;
  X3D_LevelArray face_atts;
  X3D_LevelArray line_atts;
  X3D_LevelArray texture_atts;
} X3D_Level;

typedef uint16 X3D_LEVEL_VERTEX;
typedef uint16 X3D_LEVEL_SEG;

typedef struct X3D_LevelSeg {
  uint16           flags;
  uint16           base_v;
  X3D_LEVEL_VERTEX v_start;
} X3D_LevelSeg;

X3D_LevelSeg* x3d_level_get_seg(X3D_Level* level, X3D_LEVEL_SEG seg) {
  return (X3D_LevelSeg *)level->segs.data + seg;
}

