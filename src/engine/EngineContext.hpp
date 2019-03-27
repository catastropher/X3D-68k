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

#include "engine/Config.hpp"
#include "render/Screen.hpp"
#include "dev/console/Console.hpp"
#include "render/Font.hpp"
#include "system/Time.hpp"
#include "level/BspLevel.hpp"
#include "render/OldRenderer.hpp"
#include "Init.hpp"
#include "system/Mouse.hpp"
#include "EngineQueue.hpp"
#include "entity/system/BrushModelSystem.hpp"
#include "entity/system/CameraSystem.hpp"
#include "entity/system/BoxColliderSystem.hpp"
#include "entity/system/InputSystem.hpp"

struct X_RenderContext;
class EntityManager;
class LevelManager;
class MessageQueue;

////////////////////////////////////////////////////////////////////////////////
/// A context object that holds the state for the entire engine.
////////////////////////////////////////////////////////////////////////////////
struct EngineContext
{
    Platform* getPlatform() { return &platform; }

    int frameCount;     // TODO Where should this go?
    Time frameStart;
    Time lastFrameStart;
    
    fp estimatedFramesPerSecond;
    fp timeDelta;

    Screen* screen;
    OldRenderer* renderer;
    EntityManager* entityManager;
    LevelManager* levelManager;

    Console* console;
    Font* mainFont;
    KeyState* keyState;
    MouseState* mouseState;
    EngineQueue* queue;

    MessageQueue* messageQueue;

    Platform platform;

    BrushModelSystem* brushModelSystem;
    CameraSystem* cameraSystem;
    BoxColliderSystem* boxColliderSystem;
    InputSystem* inputSystem;
};

void x_enginecontext_cleanup(EngineContext* context);
Time x_enginecontext_get_time(const EngineContext* context);
void x_enginecontext_get_rendercontext_for_camera(EngineContext* engineContext, Camera* cam, struct X_RenderContext* dest);

void x_enginecontext_restart_video(EngineContext* context);

static inline int x_enginecontext_get_frame(const EngineContext* context)
{
    return context->frameCount;
}
