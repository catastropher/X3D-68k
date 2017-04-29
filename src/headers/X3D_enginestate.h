// This file is part of X3D.
//
// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "X3D_common.h"
#include "X3D_screen.h"
#include "X3D_keys.h"
#include "X3D_player.h"
#include "X3D_render.h"
#include "X3D_init.h"
#include "X3D_object.h"
#include "memory/X3D_alloc.h"
#include "memory/X3D_handle.h"
#include "render/X3D_texture.h"

typedef struct X3D_ScreenManager {
    angle256 fov;
    int16 scale_x;
    int16 scale_y;
    
    X3D_Vex2D center;
    X3D_Vex2D pos;
    
    X3D_Texture screen;
} X3D_ScreenManager;

static inline int x3d_screenmanager_get_w(const X3D_ScreenManager* screenman) {
    return screenman->screen.w;
}

static inline int x3d_screenmanager_get_h(const X3D_ScreenManager* screenman) {
    return screenman->screen.h;
}

static inline X3D_Texture* x3d_screenmanager_get_screen(X3D_ScreenManager* screenman) {
    return &screenman->screen;
}

static inline X3D_Vex2D x3d_screenmanager_get_screen_size(const X3D_ScreenManager* screenman) {
    return x3d_vex2d_make(x3d_screenmanager_get_w(screenman), x3d_screenmanager_get_h(screenman));
}

static inline int x3d_screenmanager_total_pixels(const X3D_ScreenManager* screenman) {
    return x3d_texture_total_texels(&screenman->screen);
}

static inline int x3d_screenmanager_get_pixel_index(const X3D_ScreenManager* screenman, int x, int y) {
    return y * x3d_screenmanager_get_w(screenman) + x;
}

typedef struct X3D_EngineState {
  X3D_ScreenManager screen_manager;
  X3D_KeyManager key_manager;
  X3D_PlayerManager player_manager;
  X3D_RenderManager render_manager;
  X3D_ObjectManager object_manager;
  X3D_AllocManager alloc_manager;
  X3D_HandleManager handle_manager;
  
  _Bool exit_gameloop;
  uint16 engine_step;
} X3D_EngineState;

extern X3D_EngineState* const x3d_state;

static inline X3D_ScreenManager* x3d_screenmanager_get(void) {
  return &x3d_state->screen_manager;
}

static inline X3D_KeyManager* x3d_keymanager_get(void) {
  return &x3d_state->key_manager;
}

static inline X3D_EngineState* x3d_enginestate_get(void) {
  return x3d_state;
}

static inline X3D_RenderManager* x3d_rendermanager_get(void) {
  return &x3d_state->render_manager;
}

static inline void x3d_rendermanager_set_hud_callback(void (*callback)(void)) {
  x3d_rendermanager_get()->render_hud_callback = callback;
}

static inline void x3d_keymanager_set_callback(void (*callback)(void)) {
  x3d_keymanager_get()->key_handler = callback;
}

static inline X3D_PlayerManager* x3d_playermanager_get(void) {
  return &x3d_state->player_manager;
}

static inline uint16 x3d_enginestate_get_step(void) {
  return x3d_state->engine_step;
}

static inline void x3d_enginestate_next_step(void) {
  ++x3d_state->engine_step;
}

static inline X3D_ObjectManager* x3d_objectmanager_get(void) {
  return &x3d_state->object_manager;
}

static inline X3D_AllocManager* x3d_allocmanager_get(void) {
  return &x3d_state->alloc_manager;
}

static inline X3D_HandleManager* x3d_handlemanager_get(void) {
  return &x3d_state->handle_manager;
}

static inline void x3d_rendermanager_set_render_mode(int16 render_mode) {
    x3d_rendermanager_get()->render_mode = render_mode;
}

X3D_INTERNAL void x3d_enginestate_init(X3D_InitSettings* init);
void x3d_enginestate_cleanup(void);

