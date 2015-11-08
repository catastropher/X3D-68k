cmake_minimum_required(VERSION 2.6)

# Disable ranlib
set(CMAKE_RANLIB "echo")

# Force absolute paths
set(CMAKE_USE_RELATIVE_PATHS FALSE)

# Needed to make force the 'compiler' to tigcc-x3d
include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER($ENV{X3D}/tools/tigcc-x3d tigcc-x3d)

# Custom configuration for the archiver
set(CMAKE_AR $ENV{X3D}/tools/tigcc-x3d)
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -ar -o <TARGET> <OBJECTS> <LINK_LIBRARIES> -ar")

# Custom configuration for the linker
set(CMAKE_LINKER $ENV{X3D}/tools/tigcc-x3d)
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -n X3D")
set(CMAKE_EXECUTABLE_SUFFIX "")

# Lets the X3D source know to include the TIGCC header files
add_definitions(-D__TIGCC_HEADERS__)

if(${X3D_SUBTARGET} STREQUAL "ti92plus")
  set(CMAKE_C_FLAGS "-DUSE_TI92PLUS -O3 -Wall")
elseif(${X3D_SUBTARGET} STREQUAL "v200")
  set(CMAKE_C_FLAGS "-DUSE_V200 -O3 -Wall")
elseif(${X3D_SUBTARGET} STREQUAL "ti89")
  set(CMAKE_C_FLAGS "-DUSE_TI89 -O3 -Wall")
endif()