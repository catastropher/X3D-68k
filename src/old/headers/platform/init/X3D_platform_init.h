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
//
//
//
// X3D_platform_init.h -> functions for initializing the platform that X3D will
//    run on. This includes e.g. initializing the screen.

#pragma once

#include "common/X3D_interface.h"

struct X3D_InitSettings;
X3D_IMPLEMENTATION _Bool x3dplatform_init(struct X3D_InitSettings* init);

