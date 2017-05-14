cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-std=gnu99 -O2 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined -Wno-pointer-sign")

project(X3D C)

add_library(X3D STATIC
    src/geo/vec3.c
)

install(TARGETS X3D ARCHIVE DESTINATION /usr/local/lib)
install(DIRECTORY headers/ DESTINATION /usr/local/include/X3D FILES_MATCHING PATTERN "*.h")