// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.



// This program converts unix paths to windows paths in commands, if being run
// under Windows with Cygwin. This is necessary because the Cygwin version of
// CMake generates UNIX makefiles, but the Windows versions of GCC4TI/TIGCC expect
// Windows paths. Because X3D should be buildable on both Windows and Linux, it is
// necessary to convert them. Thus, this program converts commands in this format:
//
// tigcc-x3d ... <unix path> ... -I<unix path>
//
// to:
//
// tigcc ... <windows path> ... -I<windows path>
//
// and executes them.

#include <iostream>
#include <string>
#include <vector>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>

// Attempts to determine whether the argument is a path that needs to be
// converted to a Windows path
bool is_path(char* arg) {
  // Check if the file exists
  struct stat buf;
  
  //if(stat(arg, &buf) == 0)
  //  return true;
  
  while(*arg) {
    if(*arg++ == '/')
      return true;
  }
  
  return false;
}

int main(int argc, char* argv[]) {
#ifdef __CYGWIN__
  std::vector<char *> args;
  
  args.push_back("tigcc");
  
  fprintf(stderr,"==========================\n");
  for(int i = 1; i < argc; ++i) {
    char* new_arg = new char[1024];
    char* arg = argv[i];
    
    args.push_back(new_arg);
    
    bool quote = false;
    
    if(arg[0] == '-' && arg[1] == 'I') {
      *new_arg++ = '-';
      *new_arg++ = 'I';
      //*new_arg++ = '"';
      
      //quote = true;
      
      arg += 2;
    }
    
    if(is_path(arg)) {
        
      // Convert the path to a Windows path
      FILE* file = popen((std::string("cygpath --absolute --windows ") + std::string(arg)).c_str(), "r");
      
      if(!file) {
        std::cout << "Error: failed to execute cygpath!" << std::endl;
        std::cout << "Argument: " << arg << std::endl;
        return -1;
      }
      
      if(!fgets(new_arg, 1023, file)) {
        std::cout << "Error: could not read Windows path from cygpath!" << std::endl;
        return -1;
      }
      
      // Remove the trailing newline
      int length = strlen(new_arg);
      
      if(new_arg[length - 1] == '\n')
        --length;
      
      if(quote) {
        new_arg[length++] = '"';
      }
      
      new_arg[length] = '\0';
    }
    else {
      strcpy(new_arg, arg);
    }
  }
  
  args.push_back(NULL);
  
  fprintf(stderr,"----------------------------\n");
  for(int i = 0; i < args.size(); ++i) {
    fprintf(stderr, "%s\n", args[i]);
  }
  fprintf(stderr,"==========================\n");
  
  
  int code = execvp("tigcc", &args[0]);
  
  fprintf(stderr, "%s", strerror(errno));
  
  return code;
#else
  char name[] = "tigcc";
  
  argv[0] = name;

  execvp("tigcc", argv);
#endif
}
