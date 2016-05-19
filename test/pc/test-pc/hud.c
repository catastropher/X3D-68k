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

#include <SDL/SDL.h>

#include "X3D.h"
#include "hud.h"

extern X3D_Font font;

_Bool display_menu = 0;
_Bool display_status_bar = 0;

char hud_status_bar[512];

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

enum {
  TOOL_NONE,
  TOOL_SELECT_FACE
};

typedef struct LevelEditContext {
  X3D_SegFaceID select_face;
  int16 tool;
} LevelEditContext;

LevelEditContext edit;

void face_menu_select(void) {
  display_status_bar = X3D_TRUE;
  sprintf(hud_status_bar, "Select a face");
  edit.tool = TOOL_SELECT_FACE;
  edit.select_face = X3D_FACE_NONE;
}

void face_menu_extrude(void) {
  if(edit.tool == TOOL_SELECT_FACE && edit.select_face != X3D_FACE_NONE) {
    uint16 new_seg = x3d_segmentbuilder_add_extruded_segment(edit.select_face, 100);
    
    display_status_bar = X3D_TRUE;
    sprintf(hud_status_bar, "Added segment %d", new_seg);
  }
  else {
    display_status_bar = X3D_TRUE;
    sprintf(hud_status_bar, "No face selected!");
  }
}

HudMenu face_menu = {
  .total_items = 2,
  .items = {
    {
      .text = "[S] Select",
      .letter = 's',  
      .sub_menu = X3D_FALSE,
      .handler = face_menu_select
    },
    {
      .text = "[E] Extrude",
      .sub_menu = X3D_FALSE,
      .letter = 'e',
      .handler = face_menu_extrude
    }
  }
};

HudMenu main_menu = {
  .total_items = 3,
  .parent = NULL,
  .items = {
    {
      .text = "[S] Segment",
      .sub_menu = X3D_FALSE
    },
    {
      .text = "[F] Face",
      .sub_menu = X3D_TRUE,
      .letter = 'f',
      .menu = &face_menu
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
          if(menu->items[i].handler) {
            display_menu = X3D_FALSE;
            menu->items[i].handler();
          }
          
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
  
  for(int16 i = '0'; i <= '9'; ++i) {
    if(x3d_pc_key_down(i))
      return i;
  }
  
  if(x3d_key_down(SDLK_RETURN))
    return '\n';
  
  if(x3d_pc_key_down(SDLK_BACKSPACE))
    return '\b';
  
  return 0;
}

char menu_read_letter_key(void) {
  char k = menu_letter_key_pressed();
  
  while(menu_letter_key_pressed()) ;
  
  return k;
}

void hud_input_get_str(char* str) {
#if 0
  
  int16 pos = 0;
  
  do {
    char c = menu_read_letter_key();
    
    if(c == '\n')
      return;
    
    if(c == '\n') {
      if(pos != 0) {
        str[pos] = ' ';
        str[pos + 1] = '\0';
        
      }
    }
  }
#endif
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

void hud_handle_tools(void) {
  if(edit.tool == TOOL_SELECT_FACE) {
    _Bool left, right;
    int16 x, y;
    x3d_pc_mouse_state(&left, &right, &x, &y);
    
    if(left) {
      X3D_Vex2D mouse_pos = { x, y };
      X3D_Vex3D hit_pos;
      int16 hit_seg, hit_face;
      int16 scale;
      
      x3d_raytrace_find_segface(x3d_playermanager_get()->player[0].cam, mouse_pos, &hit_pos, &hit_seg, &hit_face, &scale);
      
      sprintf(hud_status_bar, "Selected seg: %d, face: %d", hit_seg, hit_face);
      edit.select_face = x3d_segfaceid_create(hit_seg, hit_face); 
      display_status_bar = X3D_TRUE;
    }
  }
}

void hud_render_callback(void) {
  hud_handle_tools();
  
  if(display_menu || display_status_bar) {
    hud_menu_render(current_menu);
  }
}

