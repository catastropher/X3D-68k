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

void engine_test_handle_keys(void);
void setup_key_map(void);

// Creates a hard-coded test level
void create_test_level(void) {
  // Create a new segment
  uint16 base_v = 8;
  X3D_Prism3D* prism = alloca(x3d_prism3d_size(base_v));
  X3D_Vex3D_angle256 angle = { 0, 0, 0 };

  x3d_prism3d_construct(prism, base_v, 400,  600, angle);

  x3d_rendermanager_get()->near_z = 10;
  x3d_rendermanager_get()->wireframe = X3D_FALSE;

  X3D_Polygon3D p = {
    .v = alloca(1000)
  };

  x3d_prism3d_get_face(prism, 0, &p);
  x3d_polygon3d_scale(&p, 256);
  x3d_prism3d_set_face(prism, 0, &p);

  // Create some regular segments
  uint16 id0 = x3d_segmentbuilder_add_uncompressed_segment(prism)->base.id;
  uint16 id1 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id0, 4), 100);
  uint16 id2 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id1, 4), 100);
  uint16 id3 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id1, 1), 450);
  uint16 id4 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id3, 1), 100);
  uint16 id5 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id0, 1), 2000);
  uint16 id6 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id0, 0), 100);

  // Scale the face of one of the segments
  X3D_Prism3D* p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id6)))->prism;

  x3d_prism3d_get_face(p2, 1, &p);
  x3d_polygon3d_scale(&p, 64);
  x3d_prism3d_set_face(p2, 1, &p);

  uint16 id7 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id6, 1), 400);
  uint16 id8 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id7, 1), 200);

  x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id6, 6), 500);

  p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id8)))->prism;

  x3d_prism3d_get_face(p2, 1, &p);

  // Create the pipe in the ceiling by rotating the face
  X3D_Vex3D center;
  x3d_prism3d_center(p2, &center);
  x3d_polygon3d_rotate(&p, (X3D_Vex3D_angle256) { ANG_30, 0, 0 }, center);
  x3d_prism3d_set_face(p2, 1, &p);


  // Pipe piece 2
  uint16 id9 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id8, 1), 200);

  p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id9)))->prism;

  x3d_prism3d_get_face(p2, 1, &p);

  x3d_prism3d_center(p2, &center);
  x3d_polygon3d_rotate(&p, (X3D_Vex3D_angle256) { ANG_30, 0, 0 }, center);
  x3d_prism3d_set_face(p2, 1, &p);

  // Pipe piece 3
  uint16 id10 = x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id9, 1), 500);

  p2 = &((X3D_UncompressedSegment* )(x3d_segmentmanager_get_internal(id10)))->prism;

  x3d_prism3d_get_face(p2, 1, &p);

  x3d_prism3d_center(p2, &center);
  x3d_polygon3d_rotate(&p, (X3D_Vex3D_angle256) { ANG_30, 0, 0 }, center);
  x3d_prism3d_set_face(p2, 1, &p);


  // Another segment
  x3d_segmentbuilder_add_extruded_segment(x3d_segfaceid_create(id3, 5), 200);
  
  
  // Create a new texture and attach it to the wall
  X3D_Polygon2D texture = {
    .v = alloca(2000),
    .total_v = 0
  };

  X3D_Polygon3D texture3d = {
    .v = alloca(2000)
  };

  X3D_UncompressedSegment* ss = x3d_segmentmanager_get_internal(id0);

  int16 face_id = 8;
  uint16 top_left_v = face_id - 2;
  uint16 bottom_right_v = ((top_left_v + 1) % ss->prism.base_v) + ss->prism.base_v;

  X3D_Mat3x3 mat;

  // Project the 2D polygon onto the wall
  X3D_Plane plane;
  x3d_prism3d_get_face(&ss->prism, face_id, &p);
  x3d_plane_construct(&plane, p.v, p.v + 1, p.v + 2);

  x3d_polygon2d_to_polygon3d(
    &texture,
    &texture3d,
    &plane,
    ss->prism.v + top_left_v,
    ss->prism.v + bottom_right_v,
    &mat
  );

  // Actually attach the texture
  X3D_Handle h = x3d_handle_add(&texture3d);
  x3d_uncompressedsegment_get_faces(ss)[face_id].texture = h;
  
  // Create a red and green portal
  uint16 portal_base_v = 8;
  X3D_Polygon2D portal_poly = {
    .v = alloca(sizeof(X3D_Vex3D) * 20)
  };

  x3d_polygon2d_construct(&portal_poly, portal_base_v, 60, 0);

  uint16 portal_green = x3d_wallportal_add(x3d_segfaceid_create(0, 3), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 5000);
  uint16 portal_red = x3d_wallportal_add(x3d_segfaceid_create(id0, 7), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 31);

  x3d_wallportal_connect(portal_red, portal_green);
  x3d_wallportal_connect(portal_green, portal_red);
}

// Sets up the camera for player 1
void setup_camera(void) {
  x3d_camera_init();
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;

  cam->base.base.pos = (X3D_Vex3D_fp16x8) { 0, 0, 0 };
  cam->base.angle = (X3D_Vex3D_angle256) { 0, 0, 0 };
  x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
}

void engine_test(void) {
#if defined(__linux__)
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

  // Set up key mapping
  setup_key_map();
  x3d_keymanager_set_callback(engine_test_handle_keys);

  create_test_level();
  setup_camera();
  
  x3d_game_loop();

  x3d_cleanup();
}

int main() {
  engine_test();
  return;
}

