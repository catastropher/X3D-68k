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

#include "X_Console.h"
#include "engine/X_Engine.h"
#include "util/X_util.h"

static void cmd_echo(X_EngineContext* context, int argc, char* argv[])
{
    for(int i = 1; i < argc; ++i)
        x_console_printf(&context->console, "%s ", argv[i]);
    
    x_console_print(&context->console, "\n");
}

static void cmd_map(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage: map [filename] -> loads a map file\n");
        return;
    }
    
    char fileName[512];
    strcpy(fileName, argv[1]);
    x_set_default_file_extension(fileName, ".bsp");
    
    if(x_engine_level_is_loaded(context))
    {
        x_console_print(&context->console, "A map is already loaded\n");
        return;
    }
    
    if(!x_bsplevel_load_from_bsp_file(x_engine_get_current_level(context), fileName))
    {
        x_console_printf(&context->console, "Failed to load map %s\n", fileName);
        return;
    }
    
    x_console_printf(&context->console, "Loaded map %s\n", fileName);
}

void x_console_register_builtin_commands(X_Console* console)
{
    static X_ConsoleCmd cmdEcho = { "echo", cmd_echo };
    x_console_register_cmd(console, &cmdEcho);
    
    static X_ConsoleCmd cmdMap = { "map", cmd_map };
    x_console_register_cmd(console, &cmdMap);
}
