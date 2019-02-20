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
#include "dev/console/Console.hpp"

// geo
#include "geo/BoundBox.hpp"
#include "geo/BoundSphere.hpp"
#include "geo/Frustum.hpp"
#include "geo/Plane.hpp"
#include "geo/Polygon3.hpp"
#include "geo/Ray3.hpp"
#include "geo/Vec2.hpp"
#include "geo/Vec3.hpp"
#include "geo/Vec4.hpp"

// engine
#include "engine/Config.hpp"
#include "engine/Engine.hpp"
#include "engine/EngineContext.hpp"
#include "engine/Init.hpp"

// entity
#include "entity/component/BoxColliderComponent.hpp"
#include "entity/component/CameraComponent.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityDictionary.hpp"
#include "entity/EntityManager.hpp"
#include "entity/component/InputComponent.hpp"
#include "entity/component/TransformComponent.hpp"

// error
#include "error/Error.hpp"
#include "error/Log.hpp"

// level
#include "level/BspLevel.hpp"
#include "level/BspLevelLoader.hpp"
#include "level/BspRayTracer.hpp"
#include "level/EntityModel.hpp"
#include "level/EntityModelLoader.hpp"
#include "level/LevelManager.hpp"
#include "level/Portal.hpp"

// math
#include "math/Angle.hpp"
#include "math/FixedPoint.hpp"
#include "math/Mat4x4.hpp"
#include "math/Quaternion.hpp"
#include "math/FastSqrt.hpp"
#include "math/Trig.hpp"

// memory
#include "memory/Alloc.h"
#include "memory/Array.hpp"
#include "memory/Cache.h"
#include "memory/CircularQueue.hpp"
#include "memory/OldLink.hpp"
#include "memory/Memory.hpp"
#include "memory/String.h"

// object
#include "render/Camera.hpp"

// physics
#include "physics/BoxCollider.hpp"

// platform
#include "platform/Platform.hpp"

// render
#include "render/ActiveEdge.hpp"
#include "render/Font.hpp"
#include "render/Palette.hpp"
#include "render/RenderContext.hpp"
#include "render/OldRenderer.hpp"
#include "render/Screen.hpp"
#include "render/Span.hpp"
#include "render/StatusBar.hpp"
#include "render/Texture.hpp"
#include "render/Viewport.hpp"

// system
#include "system/File.hpp"
#include "system/FilePath.hpp"
#include "system/FileReader.hpp"
#include "system/FileSystem.hpp"
#include "system/Keys.hpp"
#include "system/Mouse.hpp"
#include "system/PackFile.hpp"
#include "system/Time.hpp"

// util
#include "util/Json.hpp"
#include "util/Util.hpp"
#include "util/StopWatch.hpp"

#ifdef X_SDL_SUPPORT
#include "util/Sdl.hpp"
#endif

