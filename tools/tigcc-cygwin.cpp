#include <iostream>
#include <string>

// Attempts to determine whether the argument is a path that needs to be
// converted to a Windows path
bool is_path(char* arg) {
  // Check if the file exists
  struct stat buf;
  return stat(arg, &buf) == 0;
}

int main(int argc, char* argv[]) {
  std::string new_exec = "tigcc";
  char path[2048];
  
  for(int i = 0; i < argc; ++i) {
    if(is_path(argv[i])) {
      // Convert the path to a Windows path
      FILE* file = popen((std::string("cygpath") + " " + std::string(argv[i])).c_str(), "r");
      
      if(!file) {
        std::cout << "Error: failed to execute cygpath!" << std::endl;
        std::cout << "Argumet: " << argv[i] << std::endl;
        return -1;
      }
      
      if(!fgets(path, sizeof(path) - 1, file)) {
        std::cout << "Error: could not read Windows path from cygpath!" << std::endl;
        return -1;
      }
      
      new_exec += " " + std::string(path);
    }
    else {
      new_exec += " " + std::string(argv[i]);
    }
  }
  
  std::cout << "Command: " << new_exec << std::endl;
}