# X3D-68k
A 3D portal-rendering game engine for the TI68k graphing calculators (TI92+/TI89/Voyage 200)

## Directory Structure
  * docs
    * Doxygen configuration file for generating documentation
  * src
    * source
      * C source files
      * Makefile
      * TIGCC
        * Build of X3D for TIGCC
      * GCC
        * Build of X3D for GCC
    * headers
      * C header files
  * test
    * Test suite
    * TIGCC
      * Build of test suite for TIGCC
    * GCC
      * Build of test suite for GCC
    * Game
      * Simple test game for X3D

## Dependencies
To build X3D, you will need:
  1. GNU Make
  2. GCC
  3. TIGCC or GCC4TI (available here: http://tigcc.ticalc.org/)
  4. Extgraph by Lionel Debroux (available here: https://github.com/debrouxl/ExtGraph/tree/master/lib): copy extgraph.h to
  $TIGCC/Include/C/extgraph and extgraph.a to $TIGCC/Lib
  5. Doxygen (for generating documentation)
  
## Build Instructions
  1. Clone the X3D-68k repository!
  2. Set the environment variable $TIGCC to your TIGCC/GCC4TI directory e.g. on Windows under Cygwin you'd maybe type
  
  ```
  export TIGCC="C:\program files (x86)\TIGCC"
  ```
  3. From the X3D-68k folder, execute the following commands:
  
  ```
  cd src/source
  make clean && make && make test
  ```
  
This will build the library with TIGCC for the calculators and GCC, the test suite, and the test programs. In addition, it
will run the test suite. Note: we recommend rebuilding the entire project when a header file is changed because the build
system doesn't currently do proper dependency tracking.
