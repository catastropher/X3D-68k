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
#include "X3D_fix.h"
#include "X3D_segment.h"
#include "X3D_vector.h"
#include "X3D_render.h"
#include "X3D_trig.h"
#include "X3D_clip.h"
#include "X3D_frustum.h"
#include "X3D_engine.h"
#include "X3D_memory.h"

#ifdef __TIGCC__
#include "extgraph.h"
#endif

/// @todo document
void x3d_draw_line_black(X3D_ViewPort* context, X3D_Vex2D_int16* v1, X3D_Vex2D_int16* v2) {
#ifdef __TIGCC__
  //DrawLine(v1.x, v1.y, v2.x, v2.y, A_NORMAL);
  FastLine_Draw_R(context->screen, v1->x, v1->y, v2->x, v2->y);
#endif
}

/// @todo rename context_x and context_y to pos_x and pos_y
/// @todo document
void x3d_rendercontext_init(X3D_ViewPort* context, uint8* screen, uint16 screen_w, uint16 screen_h, uint16 context_w,
  uint16 context_h, uint16 context_x, int16 context_y, uint8 fov, uint8 flags) {

  X3D_STACK_TRACE;

  context->screen = screen;
  context->screen_w = screen_w;
  context->screen_h = screen_h;

  context->w = context_w;
  context->h = context_h;
  context->pos.x = context_x;
  context->pos.y = context_y;

  context->fov = fov;
  context->flags = flags;

  // Default center of the render context
  context->center.x = context->pos.x + context->w / 2;
  context->center.y = context->pos.y + context->h / 2;

  // Calculate the screen scaling factor (distance to the near plane)
  // dist = (w / 2) / tan(fov / 2)
  context->scale = div_int16_by_fp0x16(screen_w / 2, x3d_tanfp(fov / 2));

  //printf("Scale: %d\n", context->scale);
  //ngetchx();
}

/// @todo document
void x3d_enginestate_init(X3D_EngineState* state, uint16 max_segments, uint16 seg_pool_size) {
  x3d_stack_create(&state->table.segment_data, seg_pool_size);
  x3d_list_uint16_create(&state->table.segment_offset, max_segments);

  state->render_step = 0;
  state->frame = 0;
}

/// @todo document
void x3d_enginestate_cleanup(X3D_EngineState* state) {
  x3d_free(state->table.segment_data.base);
  x3d_free(state->table.segment_offset.base);
}

/// @todo document
/// @todo make cross platform
void x3d_renderdevice_init(X3D_RenderDevice* d, uint16 w, uint16 h) {
#ifdef __TIGCC__
  d->dbuf = malloc(LCD_SIZE);   /// @todo replace with new memory management
  PortSet(d->dbuf, LCD_WIDTH - 1, LCD_HEIGHT - 1);
#endif
}

/// @todo document
void x3d_renderdevice_cleanup(X3D_RenderDevice* d) {
#ifdef __TIGCC__
  free(d->dbuf);
  PortRestore();
#endif
}

/// @todo document
void x3d_renderdevice_flip(X3D_RenderDevice* d) {
#ifdef __TIGCC__
  FastCopyScreen_R(d->dbuf, LCD_MEM);
#endif
}

/// @todo document
void x3d_rendercontext_clamp_vex2d_int16(X3D_Vex2D_int16* v, X3D_ViewPort* context) {
  if(v->x < context->pos.x)
    v->x = context->pos.x;
  else if(v->x >= context->pos.x + context->w)
    v->x = context->pos.x + context->w - 1;

  if(v->y < context->pos.y)
    v->y = context->pos.y;
  else if(v->y >= context->pos.y + context->h)
    v->y = context->pos.y + context->h - 1;
}

void x3d_test_rotate_prism3d(X3D_Prism* dest, X3D_Prism* src, X3D_Camera* cam);

extern uint16 bouncing_box;

void x3d_render_segment_wireframe(uint16 id, X3D_Frustum* frustum, X3D_Context* context, X3D_ViewPort* viewport) {  
  uint16 i;
  

  //printf("Enter %d\n", id);

  //X3D_LOG_WAIT(context, "Enter %d\n", id);

  uint16 start = x3d_get_clock();

  X3D_Segment* seg = x3d_get_segment(context, id);
  X3D_Prism* temp = alloca(sizeof(X3D_Prism) + sizeof(X3D_Vex3D_int16) * seg->prism.base_v * 2);

  seg->last_frame = context->frame;

  x3d_test_rotate_prism3d(temp, &seg->prism, context->cam);

  // Draw the prism
  uint16 select_a = 0, select_b = 0;

  if(id == context->spinner.selected_segment) {
    x3d_get_selectspinner_selected(&context->spinner, &select_a, &select_b);
  }

  x3d_draw_clipped_prism3d_wireframe(temp, frustum, viewport, select_a, select_b);

  for(i = 0; i < X3D_MAX_OBJECTS_IN_SEGMENT; ++i) {
    if(seg->objects[i] != X3D_OBJECT_NONE) {
      X3D_Event ev;

      X3D_ObjectBase* obj = x3d_get_object(context, seg->objects[i]);
      
      ev.type = X3D_EV_RENDER;
      ev.render.frustum = frustum;
      ev.render.segment = seg;
      ev.render.viewport = viewport;

      // Trigger the object's render event
      obj->event_handler(context, obj, ev);
    }
  }
  
  X3D_SegmentFace* face = x3d_segment_get_face(seg);

  X3D_Frustum* f = ALLOCA_FRUSTUM(20);
  X3D_Polygon3D* poly = ALLOCA_POLYGON3D(20);
  X3D_Polygon3D* poly_out = ALLOCA_POLYGON3D(20);
  
#if 1
  for(i = 0; i < x3d_segment_total_f(seg); ++i) {

    //X3D_LOG_WAIT(context, "FACE ID: %d\n", face[i].connect_id);
    
    if(face[i].connect_id != SEGMENT_NONE) {
      X3D_Segment* next_seg = x3d_get_segment(context, face[i].connect_id);

      if(next_seg->last_frame == context->frame)
        continue;

      x3d_prism3d_get_face(poly, temp, i);

      Vex3D cam_pos;

      x3d_object_pos((void *)context->cam, &cam_pos);

      // Calculate the distance to the face
      int16 dist = x3d_distance_to_plane(&face[i].plane, &cam_pos);

      if(dist < 0)
        continue;

      const uint16 MIN_DIST = 100;
      
      _Bool render = 0;
      
      if(dist > MIN_DIST) {

        if(x3d_clip_polygon_to_frustum(poly, frustum, poly_out)) {
          x3d_construct_frustum_from_polygon3D(poly_out, viewport, f);
          render = 1;

#if 1
          if(i == BASE_B) {
            uint16 d;

            for(d = 0; d < f->total_p; ++d) {
              f->p[d].normal = vneg16(&f->p[d].normal);
            }

          }
#endif
        }
      }
      else {
        uint16 d;

        for(d = 0; d < frustum->total_p - 1; d++) {
          f->p[d] = frustum->p[d + 1];
        }

        f->total_p = frustum->total_p - 1;
        
        render = 1;
      }

      uint16 k;

#if 0
      for(k = 0; k < poly_out->total_v; ++k) {
        uint16 next = (k + 1) % poly_out->total_v;
        X3D_Vex2D_int16 a, b;

        x3d_vex3d_int16_project(&a, &poly_out->v[k], viewport);
        x3d_vex3d_int16_project(&b, &poly_out->v[next], viewport);

        x3d_rendercontext_clamp_vex2d_int16(&a, viewport);
        x3d_rendercontext_clamp_vex2d_int16(&b, viewport);

        x3d_draw_line_black(viewport, &a, &b);
      }
#endif

      //x3d_frustum_print(f);
      
      
      if(render) {
        x3d_render_segment_wireframe(face[i].connect_id, f, context, viewport);
      }
      
#if 0
      if(face[i].connect_id == 3) {
        uint16 d;

        for(d = 0; d < f->total_p; ++d) {
          printf("%d %d %d -> %d\n", f->p[d].normal.x, f->p[d].normal.y, f->p[d].normal.z, f->p[d].d);
        }
      }
#endif
    }

  }
#endif

  context->render_clock += x3d_get_clock() - start;

}

void x3d_selectspinner_select(X3D_SelectSpinner* spinner, X3D_Context* context, uint16 segment, uint16 face) {
  X3D_Segment* s = x3d_get_segment(context, segment);

  face = x3d_single_wrap(face, s->base_v + 2);

  spinner->selected_segment = segment;
  spinner->selected_face = face;

  spinner->base_v = (face < 2 ? s->base_v : 4);

  if(face > 1) {
    uint16 v[4];

    x3d_prism3d_side_face(&s->prism, face, v);

    spinner->select_data[0] = v[0];
    spinner->select_data[1] = v[1];
    spinner->select_data[2] = v[2];
    spinner->select_data[3] = v[3];
  }

  spinner->select_a = 0;
  spinner->select_b = spinner->base_v / 2;
}

