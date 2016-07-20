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

#include "X3D_common.h"
#include "X3D_prism.h"
#include "X3D_camera.h"
#include "X3D_enginestate.h"
#include "X3D_clip.h"
#include "X3D_trig.h"
#include "X3D_collide.h"
#include "X3D_wallportal.h"
#include "X3D_portal.h"
#include "X3D_object.h"
#include "X3D_fastsqrt.h"
#include "X3D_polygon.h"
#include "level/X3D_level.h"
#include "render/X3D_font.h"
#include "render/geo/X3D_render_prism.h"
#include "render/geo/X3D_render_linetexture.h"

#include "geo/X3D_line.h"

#include <stdio.h>


extern int16 render_mode;
extern uint16 geo_render_mode;

void x3d_rendermanager_init(X3D_InitSettings* settings) {
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  X3D_ScreenManager* screenman = x3d_screenmanager_get();
  
  // Initialize the render stack
  uint32 stack_size = 600000;
  void* render_stack_mem = malloc(stack_size);

  x3d_assert(render_stack_mem);

  x3d_stack_init(&renderman->stack, render_stack_mem, stack_size);
  
  // Reset segment face render callback
  renderman->segment_face_render_callback = NULL;
  

  int16 offx = 0, offy = 0;


  screenman->w = settings->screen_w;
  screenman->h = settings->screen_h;

  // Create the raster region for the whole screen
  X3D_Vex2D screen_v[] = {
    { offx, offy },
    { settings->screen_w - offx - 1, offy },
    { settings->screen_w - offx - 1, settings->screen_h - offy - 1 },
    { offx, settings->screen_h - offy - 1}
  };

  /*_Bool region = x3d_rasterregion_construct_from_points(
    &renderman->stack,
    &renderman->region,
    screen_v,
    4
  );*/

  //x3d_assert(region);

  //x3d_log(X3D_INFO, "Region (range=%d-%d)\n", renderman->region.rect.y_range.min, renderman->region.rect.y_range.max);

#ifndef __nspire__  
  // nspire has its zbuffer allocated with the screen
  renderman->zbuf = malloc(sizeof(int16) * screenman->w * screenman->h);
#endif
  
  
  renderman->render_hud_callback = NULL;
}

void x3d_renderer_draw_segment_wireframe(X3D_Level* level, X3D_LEVEL_SEG seg_id, X3D_CameraObject* cam, X3D_Color color) {
  X3D_LevelSegment* seg = x3d_level_get_segmentptr(level, seg_id);
  X3D_Prism3D prism = { .v = alloca(1000) };
  
  x3d_levelsegment_get_geometry(level, seg, &prism);
  x3d_prism3d_render_wireframe(&prism, cam, color);
}

void x3d_renderer_draw_all_segments(X3D_Level* level, X3D_CameraObject* cam, X3D_Color color) {
  uint16 i;
  for(i = 0; i < level->segs.total; ++i)
    x3d_renderer_draw_segment_wireframe(level, i, cam, color);
}

///////////////////////////////////////////////////////////////////////////////
/// Cleans up the render manager.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_rendermanager_cleanup(void) {
  free(x3d_rendermanager_get()->stack.base);
}


X3D_Level* global_level;
extern X3D_LineTexture3D logo;
extern X3D_LineTexture3D aperture;

void test_render_callback(X3D_CameraObject* cam);

///////////////////////////////////////////////////////////////////////////////
/// Renders the scene through a camera.
///
/// @param cam  - camera to render through
///
///////////////////////////////////////////////////////////////////////////////
void x3d_render(X3D_CameraObject* cam) {
  x3d_renderer_draw_all_segments(global_level, cam, 31);
  
  test_render_callback(cam);
  
  x3d_enginestate_next_step();
}

