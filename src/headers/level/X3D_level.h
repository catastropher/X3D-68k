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

typedef uint16 X3D_LEVEL_VERTEX;
typedef uint16 X3D_LEVEL_SEG;
typedef uint16 X3D_LEVEL_VERTEX_RUN;


typedef struct X3D_LevelSeg {
  uint16               flags;
  uint16               base_v;
  X3D_LEVEL_VERTEX_RUN v;
} X3D_LevelSeg;

typedef struct X3D_LevelSegArray {
  uint16        total;
  X3D_LevelSeg* segs;
} X3D_LevelSegArray;

typedef struct X3D_LevelVertexArray {
  uint16     total;
  X3D_Vex3D* v;
} X3D_LevelVertexArray;

typedef struct X3D_LevelVertexRunArray {
  uint16            total;
  X3D_LEVEL_VERTEX* v;
} X3D_LevelVertexRunArray;

typedef struct X3D_Level {
  X3D_LevelSegArray       segs;
  X3D_LevelVertexArray    v;
  X3D_LevelVertexRunArray runs;
} X3D_Level;

X3D_LEVEL_VERTEX x3d_level_vertex_add(X3D_Level* level, X3D_Vex3D* v);
X3D_LEVEL_VERTEX_RUN x3d_level_vertex_run_add(X3D_Level* level, X3D_LEVEL_VERTEX_RUN* run, uint16 total);
void x3d_level_init(X3D_Level* level);

