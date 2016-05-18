cmake_minimum_required(VERSION 2.6)

project(test)

if(${X3D_TARGET} STREQUAL "68k")
  INCLUDE(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER(tigcc-x3d tigcc-x3d)

  add_definitions(-D__68k__)


  set(CMAKE_C_FLAGS "-std=c99")

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
  set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> $ENV{X3D}/lib/extgraph.a $ENV{X3D}/build/X3D-68k/libX3D.a -n X3D")
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
elseif(${X3D_TARGET} STREQUAL "pc")
  set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined -Wno-pointer-sign")
  set(CMAKE_STATIC_LINKER_FLAGS, "-lm")
elseif(${X3D_TARGET} STREQUAL "nspire")
  set(CMAKE_C_FLAGS "-std=c99")
  set(CMAKE_C_COMPILER nspire-gcc)
  set(CMAKE_C_LINK_EXECUTABLE nspire-ld)
  add_definitions(-D__nspire__)
elseif(${X3D_TARGET} STREQUAL "mac")
  set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -Wno-pointer-sign -I/usr/local/Cellar/sdl2/2.0.4/include -lSDL2")
endif()

include_directories($ENV{X3D}/src/headers)

add_executable(X3D-test-pc
	test.c
	fontedit.c
	keys.c
	texturepack.c
)

set_target_properties(X3D-test-pc PROPERTIES OUTPUT_NAME test-pc)

if(${X3D_TARGET} STREQUAL "pc")
  find_package(SDL REQUIRED)
  target_link_libraries(X3D-test-pc ${SDL_LIBRARY})
  target_link_libraries(X3D-test-pc $ENV{X3D}/build/X3D/libX3D.a)
  target_link_libraries(X3D-test-pc m)

elseif(${X3D_TARGET} STREQUAL "nspire")
  target_link_libraries(X3D-test-pc $ENV{X3D}/build/nX3D/libX3D.a)
  
elseif(${X3D_TARGET} STREQUAL "mac")
  find_package(SDL2 REQUIRED)
  target_link_libraries(X3D-test-pc ${SDL_LIBRARY} ${SDL2_IMAGE_LIBRARY})
  target_link_libraries(X3D-test-pc $ENV{X3D}/build/X3D/libX3D.a)
  target_link_libraries(X3D-test-pc m)
endif()
