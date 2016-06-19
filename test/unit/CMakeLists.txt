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
  
  geo/test_geo.c
  geo/test_polygon.c
  geo/test_line.c
  
  fix/test_fp.c
  fix/test_fp0x16.c
  
  math/test_math.c
  math/test_vector.c
)

set_target_properties(UnitTests PROPERTIES OUTPUT_NAME unit)

target_link_libraries(UnitTests $ENV{X3D}/build/X3D/libX3D.a -lm)


find_package(SDL REQUIRED)
target_link_libraries(UnitTests ${SDL_LIBRARY})
