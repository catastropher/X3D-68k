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
  x3d_level_run_command("addseg id=1 v=8 r=300 h=275 pos = { 800, -400, 800 }");
  x3d_level_run_command("connect_close s1=0 s2=1");

  uint16 v = 4;

  
  X3D_Polygon2D poly = { .v = alloca(1000) };
  
  uint16 w = 50, h = 100;
  
  int16 shift = -35;
  
  //x3d_polygon2d_construct(&poly, v, 50, 0);
  
#if 1
  poly.v[0].x = -w;
  poly.v[0].y = h + shift;
  
  poly.v[1].x = w;
  poly.v[1].y = h + shift;
  
  poly.v[2].x = w;
  poly.v[2].y = -h + shift;
  
  poly.v[3].x = -w;
  poly.v[3].y = -h + shift;
  
  poly.total_v = v;
#endif
  
  
#if 1
  uint16 k;
  for(k = 0; k < 2; ++k)
    X3D_SWAP(poly.v[k], poly.v[v - k - 1]);
#endif
  
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
  
  x3d_log(X3D_INFO, "New seg: %d", new_seg->base.id);
  
  uint16 i;
  for(i = 0; i < 2; ++i) {
    //x3d_prism3d_get_face(&new_seg->prism, 2 * i + 2, &f);
    //x3d_polygon3d_scale(&f, 1024);
    //x3d_prism3d_set_face(&new_seg->prism, 2 * i + 2, &f);
  }
  
  x3d_segment_make_door(id);
  //x3d_segment_make_door(2);
  
  x3d_rendermanager_get()->near_z = 10;
  x3d_rendermanager_get()->wireframe = X3D_FALSE;
  
  // Create a red and green portal
  uint16 portal_base_v = 8;
  X3D_Polygon2D portal_poly = {
    .v = alloca(sizeof(X3D_Vex3D) * 20)
  };

  x3d_polygon2d_construct(&portal_poly, portal_base_v, 60, 0);

  x3d_segment_update(new_seg);
  
  uint16 portal_green = x3d_wallportal_add(x3d_segfaceid_create(new_seg->base.id, 3), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 5000);
  uint16 portal_red = x3d_wallportal_add(x3d_segfaceid_create(0, 7), (X3D_Vex3D) { 0, 0, 0 }, 0xFFFF, &portal_poly, 31);
  
  x3d_wallportal_connect(portal_red, portal_green);
  x3d_wallportal_connect(portal_green, portal_red);
  
  for(i = 0; i < x3d_segmentmanager_get()->alloc.alloc_offset.size; ++i)
    x3d_segment_update(x3d_segmentmanager_load(i));
  
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

X3D_Font font = {
  .glyph_width = 15,
  .glyph_height = 14,
  
  .glyph_offset_x = 1,
  .glyph_offset_y = 0,
  
  .font_space_x = 17,
  .font_space_y = 17,
  
  .font_offset_x = 19,
  .font_offset_y = 19,
  
  .font_rows = 16,
  .font_cols = 16
};

_Bool display_menu = 0;
_Bool display_status_bar = 0;

char hud_status_bar[512];

struct HudMenu;

typedef struct HudMenuItem {
  char text[64];
  char letter;
  _Bool sub_menu;
  
  union {
    void (*handler)(void);
    struct HudMenu* menu;
  };
} HudMenuItem;

typedef struct HudMenu {
  struct HudMenu* parent;
  uint16 total_items;
  HudMenuItem items[];
} HudMenu;


HudMenu* current_menu;

void hud_menu_render(HudMenu* menu) {
  int16 height;
  uint16 i;
  char menu_text[512] = "";
  
  if(menu && display_menu) {
    for(i = 0; i < menu->total_items; ++i) {
      height += x3d_font_str_height(&font, menu->items[i].text);
      strcat(menu_text, menu->items[i].text);
      strcat(menu_text, "\n");
    }
  }
  
  if(display_status_bar) {
    strcat(menu_text, hud_status_bar);
    height += x3d_font_str_height(&font, hud_status_bar);
  }
  
  x3d_font_draw_str(&font, menu_text, 0, 479 - height);
}

void segment_menu_select(void) {
  display_menu = X3D_FALSE;
  display_status_bar = X3D_TRUE;
  sprintf(hud_status_bar, "Select a segment");
}

HudMenu segment_menu = {
  .total_items = 2,
  .items = {
    {
      .text = "[S] Select",
      .letter = 's',  
      .sub_menu = X3D_FALSE,
      .handler = segment_menu_select
    },
    {
      .text = "[N] New segment",
      .sub_menu = X3D_FALSE
    }
  }
};

HudMenu main_menu = {
  .total_items = 3,
  .parent = NULL,
  .items = {
    {
      .text = "[S] Segment",
      .letter = 's',
      .menu = &segment_menu,
      .sub_menu = X3D_TRUE
    },
    {
      .text = "[F] Face",
      .sub_menu = X3D_FALSE,
    },
    {
      .text = "[V] Vertex",
      .sub_menu = X3D_FALSE
    }
  }
};

void hud_menu_send_key(HudMenu* menu, char c) {
  if(c != 0) {
    if(c == '\b') {
      if(menu->parent) {
        current_menu = menu->parent;
      }
      
      return;
    }
    
    uint16 i;
    for(i = 0; i < menu->total_items; ++i) {
      if(menu->items[i].letter == c) {
        if(menu->items[i].sub_menu) {
          menu->items[i].menu->parent = current_menu;
          current_menu = menu->items[i].menu;
          return;
        }
        else {
          if(menu->items[i].handler)
            menu->items[i].handler();
          
          return;
        }
      }
    }
    
    sprintf(hud_status_bar, "Unknown option '%c'", c);
    display_status_bar = 1;
  }
}

char menu_letter_key_pressed(void) {
  x3d_read_keys();
  
  for(int16 i = 'a'; i <= 'z'; ++i) {
    if(x3d_pc_key_down(i)) {
      return i;
    }
  }
  
  if(x3d_pc_key_down(SDLK_BACKSPACE))
    return '\b';
  
  return 0;
}

char menu_read_letter_key(void) {
  char k = menu_letter_key_pressed();
  
  while(menu_letter_key_pressed()) ;
  
  return k;
}

_Bool menu_allow_normal_keys(void) {
  if(display_menu) {
    if(x3d_pc_key_down(SDLK_LCTRL)) {
      while(x3d_pc_key_down(SDLK_LCTRL)) x3d_read_keys();
      
      display_menu = 0;
      current_menu = NULL;
      display_status_bar = X3D_FALSE;
      
      return 1;
    }
    
    hud_menu_send_key(current_menu, menu_read_letter_key());
    
    return 0;
  }
  
  if(x3d_pc_key_down(SDLK_LCTRL)) {
    while(x3d_pc_key_down(SDLK_LCTRL)) x3d_read_keys();
    
    display_menu = 1;
    current_menu = &main_menu;
    return 0;
  }
  
  return 1;
}

void hud_render_callback(void) {
  if(display_menu || display_status_bar) {
    hud_menu_render(current_menu);
  }
}


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
  
#if 1
  if(!x3d_font_load(&font, "font.bmp")) {
    x3d_log(X3D_ERROR, "Failed to load font");
    exit(0);
  }
#endif
  
  // Set up key mapping
  setup_key_map();
  x3d_keymanager_set_callback(engine_test_handle_keys);
  x3d_rendermanager_set_hud_callback(hud_render_callback);
  
  create_test_level();
  
  setup_camera();
  
  x3d_game_loop();

  x3d_cleanup();
  
  return 0;
}
