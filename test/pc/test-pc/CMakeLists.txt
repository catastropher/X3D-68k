cmake_minimum_required(VERSION 2.6)

project(test)

set(CMAKE_C_FLAGS "-std=c99 -Wall -g -fsanitize=address -fsanitize=undefined")

include_directories($ENV{X3D}/src/headers)

add_executable(X3D-test-pc
	test.c
)

set_target_properties(X3D-test-pc PROPERTIES OUTPUT_NAME test-pc)

target_link_libraries(X3D-test-pc $ENV{X3D}/build/X3D/libX3D.a)

set(SDL2_DIR $ENV{X3D}/test/pc/test-pc)

find_package(SDL2 REQUIRED)
target_link_libraries(X3D-test-pc ${SDL2_LIBRARY})