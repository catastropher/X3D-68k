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

int main(int argc, char* argv[])
{
    FILE* file = fopen("engine.log", "rb");
    
    if(!file)
        return 0;
    
    nio_console csl;
    nio_init(&csl, NIO_MAX_COLS, NIO_MAX_ROWS, 0, 0, NIO_COLOR_WHITE, NIO_COLOR_BLACK, true);
    
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

