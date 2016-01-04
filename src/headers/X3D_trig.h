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

#include "X3D_common.h"

enum {
  ANG_0 = 0,
  ANG_30 = 21,
  ANG_45 = 32,
  ANG_60 = 42,
  ANG_90 = 64,
  ANG_180 = 128,
  ANG_270 = 192,
  ANG_360 = 256
};

fp0x16 x3d_sin(angle256 angle);

#define x3d_cos(_angle) x3d_sin(ANG_90 - (_angle))

