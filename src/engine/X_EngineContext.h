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

#include "memory/X_Factory.h"
#include "render/X_Screen.h"
#include "dev/X_Console.h"
#include "render/X_Font.h"
#include "system/X_Time.h"
#include "level/X_BspLevel.h"
#include "render/X_Renderer.h"
#include "X_init.h"
#include "system/X_Mouse.h"

struct X_RenderContext;

////////////////////////////////////////////////////////////////////////////////
/// A context object that holds the state for the entire engine.
////////////////////////////////////////////////////////////////////////////////
typedef struct X_EngineContext
{
    X_Factory gameObjectFactory;        ///< Factory for creating new game objects
    X_GameObject activeObjectHead;
    X_GameObject activeObjectTail;
    X_Screen screen;                    ///< Virtual screen
    X_Console console;
    X_Font mainFont;
    X_KeyState keystate;
    X_MouseState mouseState;
    X_BspLevel currentLevel;
    X_Renderer renderer;
    
    int frameCount;     // TODO Where should this go?
    X_Time frameStart;
    X_Time lastFrameStart;
} X_EngineContext;

void x_enginecontext_init(X_EngineContext* context, X_Config* config);
void x_enginecontext_cleanup(X_EngineContext* context);
void x_enginecontext_update_time(X_EngineContext* context);
X_Time x_enginecontext_get_time(const X_EngineContext* context);
void x_enginecontext_get_rendercontext_for_camera(X_EngineContext* engineContext, X_CameraObject* cam, struct X_RenderContext* dest);

void x_enginecontext_restart_video(X_EngineContext* context);

static inline int x_enginecontext_get_frame(const X_EngineContext* context)
{
    return context->frameCount;
}

