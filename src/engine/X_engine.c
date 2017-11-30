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

#include "X_Engine.h"
#include "error/X_error.h"
#include "system/X_File.h"
#include "error/X_log.h"
#include "render/X_RenderContext.h"
#include "geo/X_Polygon3.h"
#include "platform/X_Platform.h"
#include "game/X_Game.h"

static _Bool g_engineInitialized = 0;
static X_EngineContext g_engineContext;

static X_EngineContext* x_engine_get_context(void)
{
    return &g_engineContext;
}

static void cmd_info(X_EngineContext* engineContext, int argc, char* argv[])
{
    x_console_printf
    (
        &engineContext->console,
        "\"%s\" %d.%d\nX3D version %d.%d\nCurrent map: %s\n",
        x_game_name(),
        x_game_major_version(),
        x_game_minor_version(),
        X_MAJOR_VERSION,
        X_MINOR_VERSION,
        x_engine_level_is_loaded(engineContext) ? engineContext->currentLevel.name : "<no level loaded>"
    );
}

X_EngineContext* x_engine_init(X_Config* config)
{
    if(g_engineInitialized)
        x_system_error("Called x_engine_int() after engine already initialized");
    
    x_memory_init();
    x_filesystem_init(config->programPath);
    x_log_init();
    x_filesystem_add_search_path("../assets");
    
    X_EngineContext* engineContext = x_engine_get_context();
    x_platform_init(engineContext, config);
    x_enginecontext_init(engineContext, config);
    
    // Perform a vidrestart so that we call the client's screen initialization code
    x_console_execute_cmd(&engineContext->console, "vidrestart");
    engineContext->renderer.videoInitialized = 1;
    
    x_console_register_cmd(&engineContext->console, "info", cmd_info);
    
    g_engineInitialized = 1;
    
    return engineContext;
}

void x_engine_cleanup(void)
{
    X_EngineContext* engineContext = x_engine_get_context();
    
    x_platform_cleanup(engineContext);
    x_enginecontext_cleanup(engineContext);
    x_filesystem_cleanup();
    x_memory_free_all();
    x_log_cleanup();
    
    g_engineInitialized = 0;
}

void x_engine_render_frame(X_EngineContext* engineContext)
{
    x_renderer_render_frame(engineContext);
}

