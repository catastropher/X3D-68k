cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-std=gnu99 -O2 -fPIC -Wall -g -fsanitize=address -fsanitize=undefined")

project(X3D C)

include_directories(src/)

add_library(X3D STATIC
    # engine
    src/engine/X_EngineContext.c

    # geo
    src/geo/vec3.c
    
    # memory
    src/memory/X_Factory.c
    
    # object
    src/object/X_GameObject.c
    
    # render
    src/render/canvas.c
    src/render/texture.c
)

install(TARGETS X3D ARCHIVE DESTINATION /usr/local/lib)
install(DIRECTORY src/ DESTINATION /usr/local/include/X3D FILES_MATCHING PATTERN "*.h")