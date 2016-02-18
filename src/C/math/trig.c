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

#include <math.h>

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

#define ANGLE_000    0
#define ANGLE_015   24
#define ANGLE_030   48
#define ANGLE_045   72
#define ANGLE_060   96
#define ANGLE_090  144
#define ANGLE_180  288
#define ANGLE_270  432
#define ANGLE_360  576 // don't use this value, use ANGLE_000 instead

short GetArcTanYX(short y, short x) {
  static const unsigned short tab_tan512[144]={
     0,     6,    11,    17,    22,    28,    34,    39,    45,    50,
    56,    62,    67,    73,    79,    85,    90,    96,   102,   108,
   114,   119,   125,   131,   137,   143,   149,   155,   161,   168,
   174,   180,   186,   193,   199,   206,   212,   219,   225,   232,
   239,   246,   252,   259,   267,   274,   281,   288,   296,   303,
   311,   318,   326,   334,   342,   350,   359,   367,   375,   384,
   393,   402,   411,   420,   430,   439,   449,   459,   469,   480,
   490,   501,   512,   523,   535,   547,   559,   571,   584,   597,
   610,   624,   638,   652,   667,   683,   698,   714,   731,   748,
   766,   785,   804,   823,   844,   865,   887,   910,   933,   958,
   984,  1010,  1038,  1067,  1098,  1130,  1164,  1199,  1236,  1275,
  1317,  1360,  1407,  1456,  1508,  1564,  1624,  1688,  1757,  1831,
  1911,  1998,  2092,  2196,  2309,  2435,  2574,  2729,  2904,  3101,
  3327,  3586,  3889,  4246,  4675,  5198,  5852,  6692,  7812,  9378,
 11727, 15640, 23465, 46935};
 
 
    short angle = ANGLE_090;

    if (y==0)  return (x<0 ? ANGLE_180 : ANGLE_000);

    if (x!=0) {
        long tangent = (long)((512*(long)y)/x);
        unsigned short low, high, mid, diff=65535, newdiff;
        short sign = 1;

        if (tangent < 0) {
            sign = -1;
            tangent = abs(tangent);
        }

        // now we perform a binary search to find index of closest value
        low = 0;
        high = ANGLE_090-1;
        while (low <= high) {
            mid = (low+high)/2;
            newdiff = abs(tangent-(long)tab_tan512[mid]);
            if (newdiff < diff) {
                angle = mid;
                diff = newdiff;
                if (diff <= 2)  break;
            }

            if ((long)tab_tan512[mid] < tangent) low  = mid+1;
            else                                 high = mid-1;
        }

        if (sign==-1)  angle = ANGLE_180-angle;
    }

    if (y<0)  angle += ANGLE_180;
    if (angle >= ANGLE_360)  angle -= ANGLE_360;
    return angle;
}

angle256 x3d_atan2(short y, short x) {
  int16 a = GetArcTanYX(y, x);
  
  x3d_log(X3D_INFO, "A: %d", a);
  
  return a * ANG_360 / ANGLE_360;
}

angle256 x3d_asin(fp0x16 val) {
  int32 min_diff = 0xFFFF;
  angle256 min_diff_angle = 0;
  uint16 i;
  
  for(i = 0; i < 256; ++i) {
    fp0x16 s = x3d_sin(i);
    
    if(abs(val - s) < min_diff) {
      min_diff = abs(val - s);
      min_diff_angle = i;
    }
  }
  
  return min_diff_angle;
}

angle256 x3d_acos(fp0x16 val) {
  return ANG_90 - x3d_asin(val);
}
