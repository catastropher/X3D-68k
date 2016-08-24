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
//
//
//
// test.c -> test for PC

#include "X3D.h"

#if defined(__pc__)
#include <SDL/SDL.h>
#include <alloca.h>
#endif

#if defined(__nspire__)
#include <SDL/SDL.h>
#include <alloca.h>
#endif


void engine_test_handle_keys(void);
void setup_key_map(void);

// Sets up the camera for player 1
void setup_camera(void) {
  x3d_camera_init();
  
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;

  X3D_Vex3D center = { 0, 0, 0 };
  //x3d_prism3d_center(&x3d_segmentmanager_load(0)->prism, &center);
  
  cam->base.base.pos = (X3D_Vex3D_fp16x8) { (int32)center.x << 8, (int32)center.y << 8, (int32)center.z << 8 };
  cam->base.angle = (X3D_Vex3D_angle256) { 0, 0, 0 };
  x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
}

void segment_face_render_callback(X3D_SegmentRenderFace* face);

extern X3D_Level* global_level;;

X3D_LineTexture3D logo;
X3D_LineTexture3D aperture;

X3D_Model test_model;

void build_test_model(void) {
  X3D_Prism3D prism = { .v = alloca(1000) };
  x3d_prism3d_construct(&prism, 8, 50, 50, (X3D_Vex3D_angle256) { 0, 0, 0 });
  
  x3d_model_create_dynamically_allocated_model(&test_model);
  x3d_model_add_prism3d(&test_model, &prism, x3d_vex3d_make(0, 50 + 25, 0));
  x3d_model_add_prism3d(&test_model, &prism, x3d_vex3d_make(0, -50 - 25, 0));
  
  x3d_prism3d_construct(&prism, 8, 10, 100, (X3D_Vex3D_angle256) { 0, 0, 0 });
  x3d_model_add_prism3d(&test_model, &prism, x3d_vex3d_make(0, 0, 0));
}

void init_cube(void);

void test_render_callback(X3D_CameraObject* cam) {
    
}

void build_test_level(void) {
    static X3D_Level level;
    
    x3d_level_init(&level);
    
    X3D_Prism3D prism = { .v = alloca(1000) };
    Prism3D.construct(&prism, 8, 800, 800, (X3D_Vex3D_angle256) { 0, 0, 0 });
    x3d_level_add_new_standalone_segment(&level, &prism, 0);  
    
    global_level = &level;
}

void init() {
#if defined(__pc__) && 1
    int16 w = 640;
    int16 h = 480;
#else
    int16 w = 320;
    int16 h = 240;
#endif
  
    X3D_InitSettings init = {
        .screen_w = w,
        .screen_h = h,
        .screen_scale = 1,
        .fullscreen = X3D_FALSE,
        .fov = ANG_60
    };

    x3d_init(&init);
}

int main(int argc, char* argv[]) {
    init();
    
    build_test_model();
    x3d_rendermanager_get()->near_z = 10;
    
    build_test_level();
    
    setup_key_map();
    x3d_keymanager_set_callback(engine_test_handle_keys);
    setup_camera();
    init_cube();
    
    x3d_game_loop();

    x3d_cleanup();  
    
    return 0;
}
