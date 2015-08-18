/* This file is part of X3D.
*
* X3D is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* X3D is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with X3D. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

//=============================================================================
// Types
//=============================================================================

/// A 3D polygon with a variable number of points.
/// @note This is a variable-sized data structure!
typedef struct X3D_Polygon3D {
  uint16 total_v;         ///< Total number of vertices
  Vex3D v[0];   ///< Vertices (variable number)
} X3D_Polygon3D;

/// A 2D polygon with a variable number of points.
/// @note This is a variable-sized data structure!
typedef struct X3D_Polygon2D {
  uint16 total_v;         ///< Total number of vertices
  Vex2D v[0];   ///< Vertices (variable number)
} X3D_Polygon2D;

// Returns the size needed to store a Polygon3D with the given number of points
static inline uint16 x3d_polygon3d_needed_size(uint16 total_v) {
  return sizeof(X3D_Polygon3D) + sizeof(Vex3D) * total_v;
}

