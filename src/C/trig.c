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
#include "X3D_trig.h"

fp0x16 x3d_sin(angle256 angle) {
  /// A table of fp0x16 sin values for angle256
  static const fp0x16 sintab[256] = {
    0, 804, 1607, 2410, 3211, 4011, 4808, 5602, 6392, 7179,
    7961, 8739, 9512, 10278, 11039, 11793, 12539, 13278, 14010, 14732,
    15446, 16151, 16846, 17530, 18204, 18868, 19519, 20159, 20787, 21403,
    22005, 22594, 23170, 23732, 24279, 24812, 25330, 25832, 26319, 26790,
    27245, 27684, 28106, 28511, 28898, 29269, 29621, 29956, 30273, 30572,
    30852, 31114, 31357, 31581, 31785, 31971, 32138, 32285, 32413, 32521,
    32610, 32679, 32728, 32758, 32767, 32758, 32728, 32679, 32610, 32521,
    32413, 32285, 32138, 31971, 31785, 31581, 31357, 31114, 30852, 30572,
    30273, 29956, 29621, 29269, 28898, 28511, 28106, 27684, 27245, 26790,
    26319, 25832, 25330, 24812, 24279, 23732, 23170, 22594, 22005, 21403,
    20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 15446, 14732,
    14010, 13278, 12539, 11793, 11039, 10278, 9512, 8739, 7961, 7179,
    6392, 5602, 4808, 4011, 3211, 2410, 1607, 804, 0, -804,
    -1607, -2410, -3211, -4011, -4808, -5602, -6392, -7179, -7961, -8739,
    -9512, -10278, -11039, -11793, -12539, -13278, -14010, -14732, -15446, -16151,
    -16846, -17530, -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
    -23170, -23732, -24279, -24812, -25330, -25832, -26319, -26790, -27245, -27684,
    -28106, -28511, -28898, -29269, -29621, -29956, -30273, -30572, -30852, -31114,
    -31357, -31581, -31785, -31971, -32138, -32285, -32413, -32521, -32610, -32679,
    -32728, -32758, -32767, -32758, -32728, -32679, -32610, -32521, -32413, -32285,
    -32138, -31971, -31785, -31581, -31357, -31114, -30852, -30572, -30273, -29956,
    -29621, -29269, -28898, -28511, -28106, -27684, -27245, -26790, -26319, -25832,
    -25330, -24812, -24279, -23732, -23170, -22594, -22005, -21403, -20787, -20159,
    -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13278,
    -12539, -11793, -11039, -10278, -9512, -8739, -7961, -7179, -6392, -5602,
    -4808, -4011, -3211, -2410, -1607, -804
  };
  
  return sintab[(uint16)angle];
}


/// @todo fix formatting
/**
* Calculates the tangent of an angle.
*
* @param angle - the angle as an angle256
*
* @return The tangent of the angle in fp8x8 format
* @note If angle is ANG_90 or ANG_270, this returns @ref VERTICAL_LINE_SLOPE
*/
fp8x8 x3d_tan(angle256 angle) {
  // Prevent division by 0
  if(angle == ANG_90 || angle == ANG_270)
    return VERTICAL_LINE_SLOPE;

  return div_fp0x16_by_fp0x16(x3d_sin(angle), x3d_cos(angle));
}

