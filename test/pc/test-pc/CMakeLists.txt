cmake_minimum_required(VERSION 2.6)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{X3D}/lib)
set(CALC_NAME test)

project(test)


include_directories(${PLATFORM_DIR}/headers)
include_directories(${INTERFACE_DIR})
include_directories($ENV{X3D}/src/headers)

add_executable(X3D-test-pc
	test.c
)

set_target_properties(X3D-test-pc PROPERTIES OUTPUT_NAME test-pc)

target_link_libraries(X3D-test-pc $ENV{X3D}/lib/pc/libX3DUtil.a)
target_link_libraries(X3D-test-pc $ENV{X3D}/lib/pc/libX3DCore.a)
target_link_libraries(X3D-test-pc $ENV{X3D}/lib/pc/libX3DPlatform.a)

set(SDL2_DIR $ENV{X3D}/test/pc/test-pc)

find_package(SDL2 REQUIRED)
target_link_libraries(X3D-test-pc ${SDL2_LIBRARY})