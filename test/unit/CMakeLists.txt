cmake_minimum_required(VERSION 2.6)

project(UnitTests)

  if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0 AND NOT APPLE)
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined -Wno-pointer-sign")
  else()
    set(CMAKE_C_FLAGS "-std=c99 -Wall -g -Wno-pointer-sign")
  endif()

include_directories($ENV{X3D}/src/headers)

add_executable(UnitTests
	main.c
	stack.c
	varsizeallocator.c
	segment.c
)

set_target_properties(UnitTests PROPERTIES OUTPUT_NAME unit)

target_link_libraries(UnitTests $ENV{X3D}/build/X3D/libX3D.a)

set(SDL2_DIR $ENV{X3D}/test/unit)

find_package(SDL2 REQUIRED)
target_link_libraries(UnitTests ${SDL2_LIBRARY})
