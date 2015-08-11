// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.

#include "X3D_config.h"

uint16 x3d_init_core() {//X3D_Context* context, X3D_Config* config) {
  clrscr();
  printf("Hello from DLL!\n");
  ngetchx();
  //x3d_enginestate_init(&context->state, config->max_segments, config->seg_pool_size);
  //x3d_renderdevice_init(&context->device, config->screen_w, config->screen_h);
  //x3d_rendercontext_init(&context->context, context->device.dbuf, config->screen_w,
  //                       config->screen_h, config->context_w, config->context_h,
  //                       config->context_pos.x, config->context_pos.y, config->fov,
  //                       config->flags);
  
  // Redirect interrupt handlers
  //context->old_int_1 = GetIntVec(AUTO_INT_1);
  //context->old_int_5 = GetIntVec(AUTO_INT_5);
  
  //SetIntVec(AUTO_INT_1, new_auto_int_1);
  //SetIntVec(AUTO_INT_5, DUMMY_HANDLER);
  
  //context->quit = 0;
  
  return 0;
}

