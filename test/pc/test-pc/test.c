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

#if defined(__linux__)
#include <SDL/SDL.h>
#include <alloca.h>
#endif

#if defined(__nspire__)
#include <SDL/SDL.h>
#include <alloca.h>
#endif

_Bool x3d_level_run_command(char* str);
void x3d_level_command_init(void);

void engine_test_handle_keys(void);
void setup_key_map(void);

// Creates a hard-coded test level
void create_test_level(void) {
  x3d_level_command_init();
  x3d_level_run_command("addseg id=0 v=8 r=300 h=275 pos = { 0, 0, 0 }");
  x3d_level_run_command("addseg id=1 v=8 r=300 h=275 pos = { 800, 0, 800 }");
  x3d_level_run_command("connect_close s1=0 s2=1");

  uint16 v = 4;

  
  X3D_Polygon2D poly = { .v = alloca(1000) };
  
  uint16 w = 50, h = 100;
  
  int16 shift = -35;
  
  poly.v[0].x = -w;
  poly.v[0].y = h + shift;
  
  poly.v[1].x = w;
  poly.v[1].y = h + shift;
  
  poly.v[2].x = w;
  poly.v[2].y = -h + shift;
  
  poly.v[3].x = -w;
  poly.v[3].y = -h + shift;
  
  poly.total_v = v;
  
  uint16 k;
  for(k = 0; k < 2; ++k)
    X3D_SWAP(poly.v[k], poly.v[v - k - 1]);
  
  struct {
    X3D_Polygon3D poly;
    X3D_Vex3D v[20];
  }* data = malloc(1000);//x3d_slab_alloc(sizeof(X3D_Vex3D) * 10 + sizeof(X3D_Polygon3D));
  
  data->poly.v = data->v;
  
  
  X3D_Segment* seg = x3d_segmentmanager_load(0);
  uint16 face = 5;
  X3D_Plane plane = x3d_uncompressedsegment_get_faces(seg)[face].plane;
  X3D_Mat3x3 mat;
  
  X3D_Polygon3D f = { .v = alloca(1000) };
  x3d_prism3d_get_face(&seg->prism, face, &f);
  
  X3D_Vex3D p_center;
  x3d_polygon3d_center(&f, &p_center);
  
  x3d_polygon2d_to_polygon3d(&poly, &data->poly, &plane, &p_center, &p_center, &mat);
  
  x3d_polygon3d_translate(&data->poly, p_center);
  
  
  X3D_Prism3D* prism = alloca(1000);
  
  prism->base_v = v;
  
  x3d_prism3d_set_face(prism, X3D_BASE_A, &data->poly);
  x3d_polygon3d_translate_normal(&data->poly, &plane.normal, -100);
  x3d_polygon3d_reverse(&data->poly);
  x3d_prism3d_set_face(prism, X3D_BASE_B, &data->poly);

#if 0
  X3D_Segment* seg1 = x3d_segmentmanager_load(x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(1, 1), 50));
  
  x3d_prism3d_get_face(&seg1->prism, 1, &f);
  x3d_polygon3d_scale(&f, 128);
  x3d_prism3d_set_face(&seg1->prism, 1, &f);
#endif
  
  X3D_Segment* small_seg = x3d_segmentbuilder_add_uncompressed_segment(prism);
  uint16 id = small_seg->base.id;
  
  x3d_segment_face_attach(0, face, X3D_ATTACH_WALL_PORTAL, &data->poly, x3d_segfaceid_create(id, X3D_BASE_A));
  
  X3D_Segment* new_seg = x3d_segmentmanager_load(x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id, X3D_BASE_B), 200));
  
  uint16 i;
  for(i = 0; i < 2; ++i) {
    x3d_prism3d_get_face(&new_seg->prism, 2 * i + 2, &f);
    x3d_polygon3d_scale(&f, 1024);
    x3d_prism3d_set_face(&new_seg->prism, 2 * i + 2, &f);
  }
  
  
  x3d_rendermanager_get()->near_z = 10;
  x3d_rendermanager_get()->wireframe = X3D_FALSE;
  
  // Create a red and green portal
  uint16 portal_base_v = 8;
  X3D_Polygon2D portal_poly = {
    .v = alloca(sizeof(X3D_Vex3D) * 20)
  };

  x3d_polygon2d_construct(&portal_poly, portal_base_v, 60, 0);

  uint16 id0 = 0;
  
  uint16 portal_green = x3d_wallportal_add(x3d_segfaceid_create(0, 3), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 5000);
  uint16 portal_red = x3d_wallportal_add(x3d_segfaceid_create(id0, 7), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 31);

  x3d_wallportal_connect(portal_red, portal_green);
  x3d_wallportal_connect(portal_green, portal_red);
  
}

// Sets up the camera for player 1
void setup_camera(void) {
  x3d_camera_init();
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;

  X3D_Vex3D center;
  x3d_prism3d_center(&x3d_segmentmanager_load(0)->prism, &center);
  
  cam->base.base.pos = (X3D_Vex3D_fp16x8) { (int32)center.x << 8, (int32)center.y << 8, (int32)center.z << 8 };
  cam->base.angle = (X3D_Vex3D_angle256) { 0, 0, 0 };
  x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
}

extern X3D_Texture panel_tex;
extern X3D_Texture brick_tex;
extern X3D_Texture floor_panel_tex;
extern X3D_Texture cube_tex;
extern X3D_Texture aperture_tex;

extern uint8 panel_tex_data[];
extern uint8 wood_tex_data[];
extern uint8 floor_panel_tex_data[];
extern uint8 cube_tex_data[];
extern uint8 aperture_tex_data[];

void init_textures(void) {
  x3d_texture_from_array(&panel_tex, panel_tex_data);
  x3d_texture_from_array(&brick_tex, wood_tex_data);
  x3d_texture_from_array(&floor_panel_tex, floor_panel_tex_data);
  x3d_texture_from_array(&cube_tex, cube_tex_data);
  x3d_texture_from_array(&aperture_tex, aperture_tex_data);
}

int main() {
#if defined(__linux__) && 1
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
  
  init_textures();
  
  // Set up key mapping
  setup_key_map();
  x3d_keymanager_set_callback(engine_test_handle_keys);

  create_test_level();
  
  setup_camera();
  
  x3d_game_loop();

  x3d_cleanup();
  
  return 0;
}
