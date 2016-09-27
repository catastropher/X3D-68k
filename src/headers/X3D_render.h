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

#include "X3D_common.h"

#include "render/X3D_render_hud.h"

#include "render/geo/X3D_render_line.h"

#include "X3D_prism.h"
#include "X3D_object.h"
#include "X3D_camera.h"
#include "X3D_screen.h"
#include "memory/X3D_stack.h"
#include "X3D_clip.h"


typedef struct X3D_SegmentRenderContext {
  
} X3D_SegmentRenderContext;

typedef struct X3D_SegmentRenderFace {
  X3D_SegFaceID id;
  X3D_PolygonAttributes* att;
} X3D_SegmentRenderFace;

enum {
    X3D_RENDER_NORMAL,
    X3D_RENDER_ID_BUFFER,
    X3D_RENDER_LIGHTMAP,
    X3D_RENDER_TEXTUER_LIGHTMAP
};

typedef struct X3D_RenderContext {
    uint16 render_type;
} X3D_RenderContext;

typedef struct X3D_SegRenderContext {
    X3D_RenderContext* render_context;
    X3D_CameraObject* cam;
    X3D_Level* level;
    X3D_LevelSegment* seg;
    uint16 parent_seg_id;
    X3D_Frustum* frustum;
} X3D_SegRenderContext;

typedef struct X3D_RenderManager {
  X3D_Stack stack;
  int16 near_z;
  _Bool wireframe;
  void (*segment_face_render_callback)(X3D_SegmentRenderFace* face);
  void (*render_hud_callback)(void);
  uint16* zbuf;
} X3D_RenderManager;

struct X3D_InitSettings;

void x3d_rendermanager_init(struct X3D_InitSettings* settings);

void x3d_render(X3D_CameraObject* cam, X3D_RenderContext* context);

