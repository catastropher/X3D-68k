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

// dev
#include "dev/X_Console.h"
#include "dev/X_DemoPlayer.h"
#include "dev/X_DemoRecorder.h"

// geo
#include "geo/X_Cube.h"
#include "geo/X_Frustum.h"
#include "geo/X_Plane.h"
#include "geo/X_Polygon3.h"
#include "geo/X_Ray3.h"
#include "geo/X_Vec2.h"
#include "geo/X_Vec3.h"
#include "geo/X_Vec4.h"

// engine
#include "engine/X_config.h"
#include "engine/X_Engine.h"
#include "engine/X_EngineContext.h"
#include "engine/X_init.h"

// error
#include "error/X_error.h"
#include "error/X_log.h"

// level
#include "level/X_BspLevel.h"
#include "level/X_BspLevelLoader.h"
#include "level/X_EntityModel.h"
#include "level/X_EntityModelLoader.h"
#include "level/X_RayTracer.h"

// math
#include "math/X_angle.h"
#include "math/X_fix.h"
#include "math/X_Mat4x4.h"
#include "math/X_Quaternion.h"
#include "math/X_sqrt.h"
#include "math/X_trig.h"

// memory
#include "memory/X_alloc.h"
#include "memory/X_Cache.h"
#include "memory/X_Factory.h"
#include "memory/X_String.h"

// object
#include "object/X_CameraObject.h"
#include "object/X_CubeObject.h"
#include "object/X_GameObject.h"

// physics
#include "physics/X_BoxCollider.h"

// render
#include "render/X_activeedge.h"
#include "render/X_Canvas.h"
#include "render/X_Font.h"
#include "render/X_Palette.h"
#include "render/X_RenderContext.h"
#include "render/X_Renderer.h"
#include "render/X_Screen.h"
#include "render/X_span.h"
#include "render/X_Texture.h"
#include "render/X_Viewport.h"

// system
#include "system/X_File.h"
#include "system/X_Keys.h"
#include "system/X_PackFile.h"
#include "system/X_Time.h"

// util
#include "util/X_util.h"

#ifdef X_SDL_SUPPORT
#include "util/X_sdl.h"
#endif

