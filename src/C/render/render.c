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

void x3d_renderer_draw_segment_wireframe(X3D_LEVEL_SEG seg) {
  
}

///////////////////////////////////////////////////////////////////////////////
/// Cleans up the render manager.
///
/// @return Nothing.
///////////////////////////////////////////////////////////////////////////////
void x3d_rendermanager_cleanup(void) {
  free(x3d_rendermanager_get()->stack.base);
}

///////////////////////////////////////////////////////////////////////////////
/// Renders the scene through a camera.
///
/// @param cam  - camera to render through
///
///////////////////////////////////////////////////////////////////////////////
void x3d_render(X3D_CameraObject* cam) {
  /// @todo Pseduo position isn't needed anymore since the portal implementation was upgraded
  cam->shift = (X3D_Vex3D) { 0, 0, 0 };
  x3d_object_pos(cam, &cam->pseduo_pos);
  
  x3d_screen_zbuf_clear();
  x3d_renderer_draw_hud();
  
  
  //x3d_line3d_test(cam);  
}

