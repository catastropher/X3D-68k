cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/lib)
set(CALC_NAME test)

project(test)

include_directories($ENV{X3D}/src/headers)
include_directories($ENV{X3D}/src/headers/interface/pc)

add_executable(X3D-test-pc
	test.c
)

set_target_properties(X3D-test-pc PROPERTIES OUTPUT_NAME test-pc)

target_link_libraries(X3D-test-pc $ENV{X3D}/lib/pc/libX3DUtil.a)
target_link_libraries(X3D-test-pc $ENV{X3D}/lib/pc/libX3DCore.a)
