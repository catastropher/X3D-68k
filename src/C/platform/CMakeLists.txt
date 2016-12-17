 cmake_minimum_required(VERSION 2.6)

if(${X3D_TARGET} STREQUAL "pc")
  add_sources(
    pc/platform_screen.c
    pc/platform_init.c
    pc/platform_keys.c
    pc/platform_assert.c
  )
elseif(${X3D_TARGET} STREQUAL "nspire")
  add_sources(
    nspire/platform_screen.c
    nspire/platform_init.c
    nspire/platform_keys.c
    nspire/platform_assert.c
  )
endif()
