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

#include "engine/X_EngineContext.h"
#include "engine/X_init.h"

#define X_PLATFORM_REQUIRED

X_PLATFORM_REQUIRED void x_platform_init(X_EngineContext* engineContext, X_Config* config);
X_PLATFORM_REQUIRED void x_platform_cleanup(X_EngineContext* engineContext);

X_PLATFORM_REQUIRED void x_platform_handle_keys(X_EngineContext* engineContext);

X_PLATFORM_REQUIRED void x_platform_handle_mouse(X_EngineContext* engineContext);
X_PLATFORM_REQUIRED void x_platform_mouse_set_position(X_Vec2 pos);
X_PLATFORM_REQUIRED void x_platform_mouse_show_cursor(_Bool showCursor);

