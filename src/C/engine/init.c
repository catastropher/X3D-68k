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

#include "X3D_init.h"
#include "X3D_log.h"
#include "X3D_assert.h"
#include "X3D_enginestate.h"
#include "X3D_wallportal.h"

void x3d_init(X3D_InitSettings* settings) {
  x3d_log(X3D_INFO, "X3D init"); 
  x3d_platform_init(settings);
  x3d_enginestate_init(settings);
  
  // Init wall portals
  //x3d_wallportals_init();
}

void x3d_cleanup(void) {
  x3d_log(X3D_INFO, "X3D cleanup");
  x3d_platform_cleanup();
  x3d_enginestate_cleanup();
}

