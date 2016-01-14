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

#include "X3D_init.h"
#include "X3D_log.h"
#include "X3D_assert.h"
#include "X3D_enginestate.h"

void x3d_init(X3D_InitSettings* settings) {
  x3d_log(X3D_INFO, "X3D init");  
  x3d_enginestate_init(settings);
  x3d_platform_init(settings);
  
  // Initialize the render stack
  uint32 stack_size = 600000;
  void* render_stack_mem = malloc(stack_size);
  
  x3d_assert(render_stack_mem);
  
  X3D_RenderManager* renderman = x3d_rendermanager_get();
  
  x3d_stack_init(&renderman->stack, render_stack_mem, stack_size);
  
  // Create the raster region for the whole screen
  X3D_Vex2D screen_v[] = {
    { 0, 0 },
    { settings->screen_w, 0 },
    { settings->screen_w, settings->screen_h },
    { 0, settings->screen_h }
  };
  
  _Bool region = x3d_rasterregion_construct_from_points(
    &renderman->stack,
    &renderman->region,
    screen_v,
    4
  );
  
  x3d_assert(region);
  
  
  x3d_log(X3D_INFO, "%d\n", x3d_screenmanager_get()->h);
  
  x3d_log(X3D_INFO, "Region (range=%d-%d)\n", renderman->region.y_range.min, renderman->region.y_range.max);
  
  // Init wall portals
  x3d_wallportals_init();
}

void x3d_cleanup(void) {
  x3d_log(X3D_INFO, "X3D cleanup");
  x3d_platform_cleanup();
}

