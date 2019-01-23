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
#include "memory/Factory.h"
#include "render/Screen.hpp"
#include "dev/console/Console.hpp"
#include "render/Font.hpp"
#include "system/Time.hpp"
#include "level/BspLevel.hpp"
#include "render/Renderer.hpp"
#include "Init.hpp"
#include "system/Mouse.hpp"
#include "object/ObjectFactory.hpp"
#include "EngineQueue.hpp"

struct X_RenderContext;

////////////////////////////////////////////////////////////////////////////////
/// A context object that holds the state for the entire engine.
////////////////////////////////////////////////////////////////////////////////
typedef struct X_EngineContext
{
    void init()
    {
        queue = new EngineQueue(this);
        gameObjectFactory = new X_ObjectFactory;
        console = new Console;
        mainFont = new X_Font;
        keystate = new X_KeyState;
        mouseState = new X_MouseState;
        currentLevel = nullptr;
    }
    
    X_ObjectFactory* getGameObjectFactory() const { return gameObjectFactory; }
    X_Screen* getScreen() const { return screen; }
    Console* getConsole() const { return console; }
    X_Font* getMainFont() const { return mainFont; }
    X_KeyState* getKeyState() const { return keystate; }
    X_MouseState* getMouseState() const { return mouseState; }
    BspLevel* getCurrentLevel() const { return currentLevel; }
    X_Renderer* getRenderer() const { return renderer; }
    EngineQueue* getEngineQueue() const { return queue; }

    Platform* getPlatform() { return &platform; }

    void setCurrentLevel(BspLevel* level) { currentLevel = level; }

    X_GameObject activeObjectHead;
    X_GameObject activeObjectTail;

    int frameCount;     // TODO Where should this go?
    X_Time frameStart;
    X_Time lastFrameStart;
    
    fp estimatedFramesPerSecond;
    fp timeDelta;
    
    void* userData;

    X_Screen* screen;   // FIXME: just exposing to set up DI
    X_Renderer* renderer;
    
private:
    X_ObjectFactory* gameObjectFactory;
    Console* console;
    X_Font* mainFont;
    X_KeyState* keystate;
    X_MouseState* mouseState;
    BspLevel* currentLevel;
    EngineQueue* queue;

    Platform platform;
} X_EngineContext;

void x_enginecontext_init(X_EngineContext* context, X_Config* config);
void x_enginecontext_cleanup(X_EngineContext* context);
X_Time x_enginecontext_get_time(const X_EngineContext* context);
void x_enginecontext_get_rendercontext_for_camera(X_EngineContext* engineContext, X_CameraObject* cam, struct X_RenderContext* dest);

void x_enginecontext_restart_video(X_EngineContext* context);

static inline int x_enginecontext_get_frame(const X_EngineContext* context)
{
    return context->frameCount;
}

static inline x_fp16x16 x_enginecontext_get_time_delta(X_EngineContext* engineContext)
{
    return x_fp16x16_from_int(engineContext->frameStart - engineContext->lastFrameStart) / 1000;
}

