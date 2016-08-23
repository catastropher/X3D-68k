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
#include "X3D_polygon.h"
#include "level/X3D_levelbuilder.h"

enum X3D_LevelBuilderSelectionType {
  TYPE_NONE,
  TYPE_SEGMENT,
  TYPE_FACE,
  TYPE_VERTEX,
  TYPE_POINT_ON_FACE
} X3D_LevelBuilderSelectionType;


void x3d_levelbuilder_init(X3D_LevelBuilder* builder) {
  for(uint16 i = 0; i < X3D_LEVELBUILDER_MAX_SELECTIONS; ++i)
    builder->selections[i].type = TYPE_NONE;
}

void x3d_levelbuilder_cleanup(X3D_LevelBuilder* builder) {
  
}



