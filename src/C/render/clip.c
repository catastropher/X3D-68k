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
#include "X3D_render.h"
#include "X3D_trig.h"
#include "X3D_clip.h"
#include "X3D_screen.h"
#include "X3D_enginestate.h"
#include "memory/X3D_stack.h"
#include "X3D_assert.h"
#include "X3D_keys.h"


// This code is absolutely terrible and badly needs to be refactored... it was
// never intended to *actually* be used... annoy Michael until he refactors
// this mess.


#define LCD_WIDTH x3d_screenmanager_get()->w
#define LCD_HEIGHT x3d_screenmanager_get()->h

//#undef printf
#define printf(...) ;

#define x3d_error(args...) x3d_log(X3D_ERROR, args);

#ifndef INT16_MIN
#define INT16_MIN -32767
#endif

fp16x16 x3d_vertical_slope(X3D_Vex2D v1, X3D_Vex2D v2) {
  if(v1.y == v2.y)
    return 0;
  
  return (((int32)(v2.x - v1.x)) << 16) / (v2.y - v1.y);
}

