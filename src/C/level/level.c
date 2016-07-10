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
#include "X3D_prism.h"

void x3d_level_init(X3D_Level* level) {
  level->v.total    = 0;
  level->v.v        = NULL;
  
  level->runs.total = 0;
  level->runs.v     = NULL;
  
  level->segs.total = 0;
  level->segs.segs  = NULL;
  
  level->faces.total = 0;
  level->faces.faces = NULL;
}

void x3d_level_cleanup(X3D_Level* level) {
  free(level->segs.segs);
  free(level->runs.v);
  free(level->v.v);
}

void x3d_level_test() {
  X3D_Level level;
  x3d_level_init(&level);
  
  X3D_Prism3D* prism = alloca(1000);
  x3d_prism3d_construct(prism, 8, 400, 400, (X3D_Vex3D_angle256) { 0, 0, 0 });
  
  x3d_level_segment_add(&level, prism, 0);
  
  x3d_level_cleanup(&level);
}




