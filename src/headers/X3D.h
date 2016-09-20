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

#pragma once

#ifdef __cplusplus
extern "C" {
    
#define _Bool bool
    
#endif

#include "X3D_common.h"
#include "X3D_gameloop.h"
#include "X3D_init.h"
#include "X3D_screen.h"
#include "X3D_keys.h"
#include "X3D_assert.h"

#include "memory/X3D_freelist.h"
#include "memory/X3D_stack.h"
#include "memory/X3D_list.h"
#include "memory/X3D_varsizeallocator.h"
#include "memory/X3D_slaballocator.h"
#include "memory/X3D_alloc.h"
#include "memory/X3D_handle.h"

#include "X3D_enginestate.h"
#include "X3D_prism.h"
#include "X3D_matrix.h"
#include "X3D_trig.h"
#include "X3D_object.h"
#include "X3D_camera.h"
#include "X3D_vector.h"
#include "X3D_render.h"
#include "X3D_player.h"

#include "X3D_collide.h"
#include "X3D_wallportal.h"

#include "render/X3D_texture.h"
#include "render/X3D_font.h"
#include "collide/X3D_raytrace.h"

#include "geo/X3D_line.h"

#include "level/X3D_level_linetexture.h"

#include "render/geo/X3D_render_linetexture.h"
#include "render/geo/X3D_render_model.h"
#include "render/X3D_lightmap.h"


#ifdef __cplusplus
}
#endif
    
