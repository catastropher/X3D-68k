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

#include "dev/console/Console.hpp"
#include "engine/Engine.hpp"
#include "util/Util.hpp"
#include "system/PackFile.hpp"
#include "level/LevelManager.hpp"

static void cmd_echo(EngineContext* context, int argc, char* argv[])
{
    for(int i = 1; i < argc; ++i)
    {
        x_console_printf(context->console, "%s ", argv[i]);
    }

    x_console_print(context->console, "\n");
}

static void cmd_map(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: map [filename] -> loads a map file\n");
        return;
    }
    
    char fileName[512];
    strcpy(fileName, argv[1]);
    x_filepath_set_default_file_extension(fileName, ".bsp");
    
    // FIXME: this doesn't belong here
    x_cache_flush(&context->renderer->surfaceCache);
    
    context->levelManager->switchLevel(fileName);

    x_console_printf(context->console, "Loaded map %s\n", fileName);
}

static void cmd_packlist(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: packlist [pack file name] -> prints the filenames of the files in a pack file\n");
        return;
    }
    
    X_PackFile file;
    if(!x_packfile_read_from_file(&file, argv[1]))
    {
        x_console_printf(context->console, "Failed to read packfile %s\n", argv[1]);
        return;
    }
    
    for(int i = 0; i < file.totalEntries; ++i)
        printf("%3d. %s\n", i + 1, file.entries[i].name);
    
    x_packfile_cleanup(&file);
    
    printf("\n");
}

static void cmd_packextract(EngineContext* context, int argc, char* argv[])
{
    if(argc != 3)
    {
        x_console_print(context->console, "Usage: packextract [pack file name] [dir] -> extracts the files in the pack file to [dir]\n");
        return;
    }
    
    X_PackFile file;
    if(!x_packfile_read_from_file(&file, argv[1]))
    {
        x_console_printf(context->console, "Failed to read packfile %s\n", argv[1]);
        return;
    }
    
    if(!x_packfile_extract(&file, argv[2]))
    {
        x_console_printf(context->console, "Failed to extract packfile %s\n", argv[1]);
        return;
    }
    
    x_packfile_cleanup(&file);
}

static void cmd_searchpath(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: searchpath [path] -> adds a new search path to look in when opening a file\n");
        return;
    }
    
    // TODO: check that path exists?
    x_filesystem_add_search_path(argv[1]);
    x_console_printf(context->console, "Added search path %s\n", argv[1]);
}

static void cmd_exec(EngineContext* context, int argc, char* argv[])
{
    if(argc != 2)
    {
        x_console_print(context->console, "Usage: exec [file] -> executes a console script file\n");
        return;
    }
    
    char line[512];
    X_File file;
    
    if(!x_file_open_reading(&file, argv[1]))
    {
        x_console_printf(context->console, "Failed to open file %s for execution\n", argv[1]);
        return;
    }
    
    while(x_file_read_line(&file, sizeof(line), line))
    {
        if(*line)
        {
            x_console_execute_cmd(context->console, line);
        }
    }
}

void x_console_register_builtin_commands(Console* console)
{
    x_console_register_cmd(console, "echo", cmd_echo);    
    x_console_register_cmd(console, "map", cmd_map);    
    x_console_register_cmd(console, "packlist", cmd_packlist);    
    x_console_register_cmd(console, "packextract", cmd_packextract);    
    x_console_register_cmd(console, "searchpath", cmd_searchpath);    
    x_console_register_cmd(console, "exec", cmd_exec);
}

