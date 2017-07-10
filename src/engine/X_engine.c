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


static _Bool g_engineInitialized = 0;
static X_EngineContext g_engineContext;

static X_EngineContext* x_engine_get_context(void)
{
    return &g_engineContext;
}

X_EngineContext* x_engine_init(int screenW, int screenH, const char* programPath)
{
    if(g_engineInitialized)
        x_system_error("Called x_engine_int() after engine already initialized");
    
    x_memory_init();
    x_filesystem_init(programPath);
    x_log_init();
    x_filesystem_add_search_path("../assets");
    
    X_EngineContext* engineContext = x_engine_get_context();
    x_enginecontext_init(engineContext, screenW, screenH);
    
    return engineContext;
}

void x_engine_cleanup(void)
{
    x_enginecontext_cleanup(x_engine_get_context());
    x_filesystem_cleanup();
    x_memory_free_all();
    x_log_cleanup();
}



