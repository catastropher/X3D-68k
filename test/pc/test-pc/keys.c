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

#ifndef __nspire__
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif

#include <X3D/X3D.h>

enum {
  KEY_ENTER = X3D_KEY_0,
  KEY_W = X3D_KEY_1,
  KEY_S = X3D_KEY_2,
  KEY_A = X3D_KEY_3,
  KEY_D = X3D_KEY_4,
  KEY_ESCAPE = X3D_KEY_5,
  KEY_Q = X3D_KEY_6,
  KEY_E = X3D_KEY_7,
  KEY_UP = X3D_KEY_8,
  KEY_DOWN = X3D_KEY_9,
  KEY_1 = X3D_KEY_10,
  KEY_2 = X3D_KEY_11,
  KEY_9 = X3D_KEY_12,
  KEY_0 = X3D_KEY_13,
  KEY_RECORD = X3D_KEY_14
};


void setup_key_map(void) {
#ifdef __pc__
    x3d_key_map_pc(KEY_ENTER, SDLK_RETURN);
    x3d_key_map_pc(KEY_ESCAPE, SDLK_ESCAPE);
    x3d_key_map_pc(KEY_W, 'w');
    x3d_key_map_pc(KEY_A, 'a');
    x3d_key_map_pc(KEY_S, 's');
    x3d_key_map_pc(KEY_D, 'd');
    x3d_key_map_pc(KEY_Q, SDLK_DOWN);
    x3d_key_map_pc(KEY_E, SDLK_UP);
    x3d_key_map_pc(KEY_UP, '[');
    x3d_key_map_pc(KEY_DOWN, ']');
    x3d_key_map_pc(KEY_1, '1');
    x3d_key_map_pc(KEY_2, '2');
    x3d_key_map_pc(KEY_0, '0');
    x3d_key_map_pc(KEY_9, '9');
    x3d_key_map_pc(KEY_RECORD, 'm');
    x3d_key_map_pc(X3D_KEY_15, 'p');
#elif defined(__nspire__)
    x3d_key_map_pc(KEY_ENTER, SDLK_RETURN);
    x3d_key_map_pc(KEY_W, SDLK_7);
    x3d_key_map_pc(KEY_S, SDLK_4);
    x3d_key_map_pc(KEY_A, SDLK_LEFT);
    x3d_key_map_pc(KEY_D, SDLK_RIGHT);
    x3d_key_map_pc(KEY_E, SDLK_UP);
    x3d_key_map_pc(KEY_Q, SDLK_DOWN);
    x3d_key_map_pc(KEY_9, SDLK_3);
    x3d_key_map_pc(KEY_ESCAPE, SDLK_ESCAPE);
    x3d_key_map_pc(X3D_KEY_15, SDLK_9);
#endif
}

extern X3D_Level* global_level;
extern X3D_LightMapContext lightmap_context;

_Bool menu_allow_normal_keys(void);

extern X3D_Texture checkerboard;

void engine_test_handle_keys(void) {
  X3D_CameraObject* cam = x3d_playermanager_get()->player[0].cam;
  static _Bool rec = X3D_FALSE;

#if 0
  
  if(!menu_allow_normal_keys())
    return;

  if(x3d_key_down(KEY_ENTER)) {
    while(x3d_key_down(KEY_ENTER))
      x3d_read_keys();
    
    X3D_SegmentManager* segmentman = x3d_segmentmanager_get();
    
    uint16 i;
    for(i = 0; i < segmentman->alloc.alloc_offset.size; ++i) {
      X3D_Segment* seg = x3d_segmentmanager_load(i);
      
      if(x3d_segment_is_door(seg)) {
        if(seg->door_data->mode == X3D_DOOR_CLOSING)
          seg->door_data->mode = X3D_DOOR_OPENING;
        else
          seg->door_data->mode = X3D_DOOR_CLOSING;
      }
    }
  }
#endif


#ifdef __linux__
  if(x3d_key_down(KEY_RECORD)) {
    while(x3d_key_down(KEY_RECORD)) {
      x3d_read_keys();
    }

    if(!rec) {
      system("rm -rf ~/record");
      system("mkdir ~/record");

      printf("Begin recording in (hold M to abort):\n");

      _Bool record = X3D_TRUE;

      int16 i;
      for(i = 3; i >= 1; --i) {
        printf("%d\n", i);
        SDL_Delay(1000);

        x3d_read_keys();
        if(x3d_key_down(KEY_RECORD)) {
          printf("Recording aborted\n");
          while(x3d_key_down(KEY_RECORD)) {
            x3d_read_keys();
          }

          record = X3D_FALSE;
          break;
        }
      }

      if(record) {
        printf("Begin!\n");
        //x3d_screen_begin_record("/home/michael/record/frame");
        rec = X3D_TRUE;
      }
    }
    else {
      printf("Recording complete\n");
      //x3d_screen_record_end();
      rec = X3D_FALSE;
    }
  }
#endif

#if 0
  if(x3d_key_down(KEY_1)) {
    X3D_RayCaster caster;
    X3D_Vex3D_fp0x16 dir;

    x3d_dynamicobject_forward_vector(&cam->base, &dir);

    x3d_raycaster_init(&caster, cam->base.base.seg, cam->base.base.pos, dir);
    x3d_raycaster_cast(&caster);


    printf("Seg: %d, Face: %d, Pos: { %d, %d, %d }\n", x3d_segfaceid_seg(caster.hit_face),
      x3d_segfaceid_face(caster.hit_face), caster.hit_pos.x, caster.hit_pos.y, caster.hit_pos.z);

    // Create a portal on one of the walls
    uint16 portal_base_v = 8;
    X3D_Polygon2D portal_poly = {
      .v = alloca(sizeof(X3D_Vex2D) * 20)
    };

    x3d_polygon2d_construct(&portal_poly, portal_base_v, 60, 0);
    x3d_wallportal_construct(0, caster.hit_face, caster.hit_pos, 1, &portal_poly, 5000);
    x3d_wallportal_update(0);
    x3d_wallportal_update(1);
  }

  if(x3d_key_down(KEY_2)) {
    X3D_RayCaster caster;
    X3D_Vex3D_fp0x16 dir;

    x3d_dynamicobject_forward_vector(&cam->base, &dir);

    x3d_raycaster_init(&caster, cam->base.base.seg, cam->base.base.pos, dir);
    x3d_raycaster_cast(&caster);

    printf("Seg: %d, Face: %d, Pos: { %d, %d, %d }\n", x3d_segfaceid_seg(caster.hit_face),
      x3d_segfaceid_face(caster.hit_face), caster.hit_pos.x, caster.hit_pos.y, caster.hit_pos.z);

    // Create a portal on one of the walls
    uint16 portal_base_v = 8;
    X3D_Polygon2D portal_poly = {
      .v = alloca(sizeof(X3D_Vex2D) * 20)
    };

    x3d_polygon2d_construct(&portal_poly, portal_base_v, 60, 0);
    x3d_wallportal_construct(1, caster.hit_face, caster.hit_pos, 0, &portal_poly, 31);
    x3d_wallportal_update(0);
    x3d_wallportal_update(1);
  }
#endif

  if(x3d_key_down(KEY_ESCAPE)) {
    x3d_game_loop_quit();
  }

#if 1
  if(x3d_key_down(KEY_W)) {
    X3D_Vex3D dir;
    x3d_dynamicobject_forward_vector(&cam->base, &dir);

#if 0
    X3D_Vex3D_fp8x8 d = {
      dir.x >> 3,
      dir.y >> 3,
      dir.z >> 3
    };

    cam->base.velocity = d;
    x3d_object_move(&cam->base);
#else

    cam->base.base.pos.x += dir.x >> 3;
    cam->base.base.pos.y += dir.y >> 3;
    cam->base.base.pos.z += dir.z >> 3;

#endif

    //cam->base.base.pos.z += 4L << 8;
  }
  else if(x3d_key_down(KEY_S)) {
    X3D_Vex3D dir;
    x3d_dynamicobject_forward_vector(&cam->base, &dir);

    #if 0
    X3D_Vex3D_fp8x8 d = {
      -dir.x >> 3,
      -dir.y >> 3,
      -dir.z >> 3
    };

    cam->base.velocity = d;
    x3d_object_move(&cam->base);
#else

    cam->base.base.pos.x -= dir.x >> 3;
    cam->base.base.pos.y -= dir.y >> 3;
    cam->base.base.pos.z -= dir.z >> 3;

#endif

    //cam->base.base.pos.z += 4L << 8;
  }

#if 1
  if(x3d_key_down(KEY_Q)) {
    cam->base.angle.x -= 4;
    x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  }
  else if(x3d_key_down(KEY_E)) {
    cam->base.angle.x += 4;
    x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  }
#endif

  if(x3d_key_down(KEY_A)) {
    cam->base.angle.y -= 4;
    x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  }
  else if(x3d_key_down(KEY_D)) {
    cam->base.angle.y += 4;
    x3d_mat3x3_construct(&cam->base.mat, &cam->base.angle);
  }

  if(x3d_key_down(KEY_UP)) {
    cam->base.base.pos.y -= 4L << 8;
  }
  else if(x3d_key_down(KEY_DOWN)) {
    cam->base.base.pos.y += 4L << 8;
  }
  
  if(x3d_key_down(KEY_ENTER)) {
      X3D_SpotLight light;
        
      
      
        x3d_object_pos(cam, &light.pos);
        light.orientation.y = cam->base.angle.y;//x3d_enginestate_get_step();
        light.orientation.x = cam->base.angle.x;
        
        x3d_lightmap_build(&light, &lightmap_context);
        
        x3d_lightmapcontext_build_surfaces(&lightmap_context, &checkerboard);
        
        while(x3d_key_down(KEY_ENTER)) x3d_read_keys();
  }
  
  _Bool left = X3D_FALSE, right;
  int16 mouse_x, mouse_y;
  //x3d_pc_mouse_state(&left, &right, &mouse_x, &mouse_y);
  
  if(left) {
    X3D_Vex2D mouse_pos = { mouse_x, mouse_y };
    X3D_RayTracer raytracer;
    x3d_raytracer_init_from_point_on_screen(&raytracer, global_level, cam, mouse_pos);
    
    if(x3d_raytracer_find_closest_hit_wall(&raytracer)) {
      //x3d_log(X3D_INFO, "Hit seg %d, face %d", raytracer.hit_seg, raytracer.hit_seg_face);
      
      x3d_level_add_extruded_segment(global_level, x3d_segfaceid_create(raytracer.hit_seg, raytracer.hit_seg_face), 400);
      
      do {
        x3d_read_keys();
        x3d_pc_mouse_state(&left, &right, &mouse_x, &mouse_y);
      } while(left);
      
      uint16 j;
      X3D_LevelSegment* ss;
      
      X3D_Level level = *global_level;
      
      for(j = 0; j < level.segs.total; ++j) {
        ss = x3d_level_get_segmentptr(&level, j);
        
        x3d_log(X3D_INFO, "%d", j);
        uint16 k;
        for(k = 0; k < ss->base_v + 2; ++k) {
          uint16 face = x3d_levelsegment_get_face_attribute(&level, ss, k)->connect_face;
          
          if(face != X3D_FACE_NONE)
            x3d_log(X3D_INFO, "\t%d %d", x3d_segfaceid_seg(face), x3d_segfaceid_face(face));
          else
            x3d_log(X3D_INFO, "\tNONE");
        }
      }
    }
  }
  
  if(x3d_pc_key_down('i')) {
      x3d_rendermanager_set_render_mode(X3D_RENDER_TEXTUER_LIGHTMAP);
      x3d_log(X3D_INFO, "Set render mode");
  }
  
  
#endif
}
