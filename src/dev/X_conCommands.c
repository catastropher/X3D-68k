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
#include "system/X_PackFile.h"
#include "level/X_BspLevelLoader.h"

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

static void cmd_packlist(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage: packlist [pack file name] -> prints the filenames of the files in a pack file\n");
        return;
    }
    
    X_PackFile file;
    if(!x_packfile_read_from_file(&file, argv[1]))
    {
        x_console_printf(&context->console, "Failed to read packfile %s\n", argv[1]);
        return;
    }
    
    for(int i = 0; i < file.totalEntries; ++i)
        printf("%3d. %s\n", i + 1, file.entries[i].name);
    
    x_packfile_cleanup(&file);
    
    printf("\n");
}

static void cmd_packextract(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 3)
    {
        x_console_print(&context->console, "Usage: packextract [pack file name] [dir] -> extracts the files in the pack file to [dir]\n");
        return;
    }
    
    X_PackFile file;
    if(!x_packfile_read_from_file(&file, argv[1]))
    {
        x_console_printf(&context->console, "Failed to read packfile %s\n", argv[1]);
        return;
    }
    
    if(!x_packfile_extract(&file, argv[2]))
    {
        x_console_printf(&context->console, "Failed to extract packfile %s\n", argv[1]);
        return;
    }
    
    x_packfile_cleanup(&file);
}

static void cmd_searchpath(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage: searchpath [path] -> adds a new search path to look in when opening a file\n");
        return;
    }
    
    // TODO: check that path exists?
    x_filesystem_add_search_path(argv[1]);
    x_console_printf(&context->console, "Added search path %s\n", argv[1]);
}

static void cmd_exec(X_EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(&context->console, "Usage: exec [file] -> executes a console script file\n");
        return;
    }
    
    char line[512];
    X_File file;
    
    if(!x_file_open_reading(&file, argv[1]))
    {
        x_console_printf(&context->console, "Failed to open file %s for execution\n", argv[1]);
        return;
    }
    
    while(x_file_read_line(&file, sizeof(line), line))
    {
        if(*line)
            x_console_execute_cmd(&context->console, line);
    }
}

void x_console_register_builtin_commands(X_Console* console)
{
    static X_ConsoleCmd cmdEcho = { "echo", cmd_echo };
    x_console_register_cmd(console, &cmdEcho);
    
    static X_ConsoleCmd cmdMap = { "map", cmd_map };
    x_console_register_cmd(console, &cmdMap);
    
    static X_ConsoleCmd cmdPacklist = { "packlist", cmd_packlist };
    x_console_register_cmd(console, &cmdPacklist);
    
    static X_ConsoleCmd cmdPackextract = { "packextract", cmd_packextract};
    x_console_register_cmd(console, &cmdPackextract);
    
    static X_ConsoleCmd cmdSearchpath = { "searchpath", cmd_searchpath };
    x_console_register_cmd(console, &cmdSearchpath);
    
    static X_ConsoleCmd cmdExec = { "exec", cmd_exec };
    x_console_register_cmd(console, &cmdExec);
}

