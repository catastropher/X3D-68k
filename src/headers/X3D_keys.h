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

#pragma once

#include "X3D_fix.h"

/// A custom key, which holds the row and column in the keypad matrix.
typedef struct X3D_CustomKey {
  uint8 row;			///< Row
  uint8 col;			///< Column
} X3D_CustomKey;

/// A custom key mapper that maps an X3D key to a keymatrix 
typedef struct X3D_KeyMapper {
  X3D_CustomKey keys[16];
} X3D_KeyMapper;

static inline void x3d_keymapper_map(X3D_KeyMapper* mapper, uint16 x3d_key, uint8 row, uint8 col) {
  mapper->keys[x3d_key].row = row;
  mapper->keys[x3d_key].col = col;
}