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
#include "X3D_init.h"

#include "X3D_platform_init.h"

/// @todo this should return whether initialization succeeded
X3D_INTERNAL void x3d_platform_init(X3D_InitSettings* settings) {
  x3d_platform_screen_init(settings);
  x3d_platform_keys_init(settings);
}

X3D_PLATFORM
X3D_INTERNAL
void x3d_platform_cleanup(void) {
  x3d_platform_screen_cleanup();
}