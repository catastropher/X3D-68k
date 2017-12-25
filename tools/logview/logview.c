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

// Prints out the log on the Nspire (useful for when X3D's initialization fails)
// The text editor on the calc can't open the log file, so this program lets you read it

#include <nspireio/nspireio.h>
#include <stdio.h>
#include <string.h>
#include <libndls.h>

void filepath_extract_path(const char* filePath, char* path)
{
    const char* str = filePath + strlen(filePath) - 1;
    while(str != filePath && *str != '/')
    {
        --str;
    }
    
    while(filePath < str)
    {
        *path++ = *filePath++;
    }
    
    *path = '\0';
}

int main(int argc, char* argv[])
{
    char logPath[256];
    filepath_extract_path(argv[0], logPath);
    strcat(logPath, "/engine.log");
    
    FILE* file = fopen(logPath, "rb");
    
    if(!file)
    {
        show_msgbox("Error", "engine.log not found");
        return 0;
    }
    
    nio_console csl;
    nio_init(&csl, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true);
    
    nio_fputs("log:\n", &csl);
    
    int c;
    while((c = fgetc(file)) != EOF)
    {
        char str[2] = { c, '\0'};
        
        nio_fputs(str, &csl);
    }
    
    fclose(file);
    nio_free(&csl);
    wait_key_pressed();
    
    return 0;
}

