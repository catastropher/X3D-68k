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

#include "X3D_vector.h"

/// A plane described by the equation ax + by + cz - d = 0, where <a, b, c>
/// is the surface normal and d is the perpendicular distance from the origin.
typedef struct X3D_Plane {
  Vex3D_fp0x16 normal;    ///< Surface normal (a vector perpendicular to the plane)
  int16 d;                    ///< Perpendicular distance from the origin
} X3D_Plane;

