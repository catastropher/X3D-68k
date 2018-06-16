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

#include <X3D/X3D.h>

#include "init.h"
#include "screen.h"
#include "keys.h"

void init(Context* context, const char* programPath, X_Config& config)
{
    //init_x3d(context, resolution.x, resolution.y, programPath);    
    //init_camera(context);    
    init_keys(context);

    screen_set_callbacks(context, *config.screen);
    
    x_console_execute_cmd(context->engineContext->getConsole(), "searchpath ..");
    x_console_execute_cmd(context->engineContext->getConsole(), "exec ../engine.cfg");
    
    screen_init_console_vars(context->engineContext->getConsole());
    
    context->quit = 0;
    context->netMode = NET_CLIENT;
}

static void cleanup_sdl(Context* context)
{
    // Make sure we don't quit without putting the screen back to normal
    SDL_SetVideoMode(context->nativeResolutionW, context->nativeResolutionH, 32, SDL_SWSURFACE);
    
    SDL_Quit();
}

static void cleanup_x3d(Context* context)
{
    x_engine_cleanup();
}

void cleanup(Context* context)
{
    cleanup_keys(context);
    cleanup_sdl(context);
    cleanup_x3d(context);
}

