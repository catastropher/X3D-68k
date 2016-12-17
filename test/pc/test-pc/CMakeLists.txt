cmake_minimum_required(VERSION 2.6)

project(test)

if(${X3D_TARGET} STREQUAL "pc")
  if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0 AND NOT APPLE)
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined -Wno-pointer-sign")
  else()
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -Wno-pointer-sign")
  endif()
 
  set(CMAKE_STATIC_LINKER_FLAGS, "-lm")
  add_definitions(-D__pc__)
elseif(${X3D_TARGET} STREQUAL "nspire")
  set(CMAKE_C_FLAGS "-std=c99")
  set(CMAKE_C_COMPILER nspire-gcc)
  set(CMAKE_C_LINK_EXECUTABLE nspire-ld)
  add_definitions(-D__nspire__)
endif()

include_directories(/usr/local/include/X3D)

add_executable(X3D-test-pc
  test.c
  fontedit.c
  keys.c
  hud.c
  cube.c
)

set_target_properties(X3D-test-pc PROPERTIES OUTPUT_NAME test-pc)

if(${X3D_TARGET} STREQUAL "pc")
  find_package(SDL2 REQUIRED)
  target_link_libraries(X3D-test-pc X3D)
  target_link_libraries(X3D-test-pc SDL2)
  target_link_libraries(X3D-test-pc m)

elseif(${X3D_TARGET} STREQUAL "nspire")
  target_link_libraries(X3D-test-pc X3D)
endif()
