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

static inline void x3d_mat3x3_set_column(X3D_Mat3x3* mat, int16 col, X3D_Vex3D_fp0x16* v) {
  mat->data[col] = v->x;
  mat->data[col + 3] = v->y;
  mat->data[col + 6] = v->z;
}

static inline void x3d_mat3x3_get_column(X3D_Mat3x3* mat, int16 col, X3D_Vex3D_fp0x16* dest) {
  dest->x = mat->data[col];
  dest->y = mat->data[col + 3];
  dest->z = mat->data[col + 6];
}

static inline void x3d_mat3x3_set_row(X3D_Mat3x3* mat, int16 row, X3D_Vex3D_fp0x16* v) {
  mat->data[row * 3] = v->x;
  mat->data[row * 3 + 1] = v->y;
  mat->data[row * 3 + 2] = v->z;
}

static inline void x3d_mat3x3_get_row(X3D_Mat3x3* mat, int16 row, X3D_Vex3D_fp0x16* dest) {
  dest->x = mat->data[row * 3];
  dest->y = mat->data[row * 3 + 1];
  dest->z = mat->data[row * 3 + 2];
}

