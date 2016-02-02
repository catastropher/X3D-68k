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

#include "X3D_common.h"
#include "X3D_vector.h"

#if 0

typedef struct X3D_BoundRect {
  X3D_Vex2D v[2];
} X3D_BoundRect;

typedef struct ClipEdge {  
  X3D_Vex3D start;
  X3D_Vex3D end;
  
  uint8 flags;
  
  int16* x_data;
  
  X3D_Range x_range;
  X3D_Range y_range;
  
} X3D_ClipEdge;

typedef struct X3D_ClipRegion {
  X3D_BoundRect rect;
  
  union {
    int16* x_left;
    uint8* x_left_small;
  };
    
  union {
    int16* x_right;
    uint8* x_right_small;
  };
} X3D_ClipRegion;

#endif

