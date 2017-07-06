# X3D
A cross-platform 3D software rendering game engine!

## Dependencies for Building X3D
You will need:
  1. GNU Make
  2. CMake
  3. gcc or clang (a version with gnu99 support)
  4. libasan for debugging runtime errors

## Library Build Instructions
```
git clone https://github.com/catastropher/X3D-68k.git
cd X3D-68k
mkdir build
cd build
cmake ..
make
sudo make install
```
## Dependencies for Test Project
You will need:
  1. X3D (see build instructions above)
  2. libsdl1.2-dev
  3. libasan

## Test Project Build Instructions
```
cd test
mkdir build
cd build
cmake ..
make
./xtest
```

Note: If you make a change in X3D, make sure to run `sudo make install` afterwards AND do a clean build of xtest to relink the library
