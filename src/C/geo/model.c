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

#include "X3D_vector.h"
#include "X3D_screen.h"

typedef struct X3D_ModelVertex {
  X3D_Vex3D v;
} X3D_ModelVertex;

typedef struct X3D_ModelEdge {
  uint16 v[2];
} X3D_ModelEdge;

typedef struct X3D_ModelFace {
  uint16 total_e;
  uint16* e;
  X3D_Color color;
} X3D_ModelFace;

typedef struct X3D_Model {
  uint16 total_v;
  X3D_ModelVertex* v;
} X3D_Model;


