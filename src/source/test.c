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

// Everything in this file should be considered experimental!

#include "X3D_config.h"
#include "X3D_fix.h"
#include "X3D_vector.h"
#include "X3D_geo.h"
#include "X3D_clip.h"
#include "X3D_render.h"
#include "X3D_frustum.h"
#include "X3D_trig.h"
#include "X3D_segment.h"
#include "X3D_matrix.h"
#include "X3D_error.h"

#ifdef __TIGCC_HEADERS__
#include <tigcclib.h>
#endif

enum {
  INSIDE,
  BOUNDARY,
  OUTSIDE
};

/// @todo This should be reorganized into an array of two vertices instead of separate arrays
typedef struct {
  _Bool vis;
  uint16 clip_edge[2];
  X3D_Vex2D_int16 v[2];
} Edge;



_Bool clip_line(X3D_ClipRegion* clip, Edge* edge) {
  // List of PL id's that each vertex is outside of
  int16 outside[2][clip->total_pl];
  
  // Position in the outside list for each vertex
  int16 outside_pos[2] = {0, 0};
  
  // Distance between each vertex and each line
  int32 dist[2][clip->total_pl];
  
  uint16 line, vertex, i;
  
  // Calculate the distance between each vertex and each line
  for(line = 0; line < clip->total_pl; ++line) {
    for(vertex = 0; vertex < 2; ++vertex) {
      dist[vertex][line] = (int32)clip->pl[line].normal.x * edge->v[vertex].x + (int32)clip->pl[line].normal.y * edge->v[vertex].y - clip->pl[line].d;
    }
    
    if(dist[0][line] < 0) {
      if(dist[1][line] < 0) {
        // If both vertices fail by the same line, the line is invisible
        return 0;
      }
      else {
        outside[0][outside_pos[0]++] = line;
      }
    }
    else if(dist[1][line] < 0) {
      outside[1][outside_pos[1]++] = line;
    }
  }
  
  // If both points are totally inside, the line is visible
  if((outside_pos[0] | outside_pos[1]) == 0)
    return 1;
    
  // Minumum scale needed to scale the line by to get the point inside the boundary
  int16 min_scale[2] = {0x7FFF, 0x7FFF};
  X3D_Vex2D_int16 clipped[2] = {edge->v[0], edge->v[1]};
    
  // We now have a list of lines that each vertex fails against
  for(vertex = 0; vertex < 2; ++vertex) {
    if(outside_pos[vertex] != 0) {
      int16 min_scale_index = 0;
      
      for(i = 0; i < outside_pos[vertex]; ++i) {
        int16 scale = ((int32)abs(dist[vertex][outside[vertex][i]]) << 8) / (abs(dist[vertex ^ 1][outside[vertex][i]]) + abs(dist[vertex][outside[vertex][i]]));
        
        if(scale < min_scale[vertex]) {
          min_scale[vertex] = scale;
          min_scale_index = outside[vertex][i];
        }
      }
      
      clipped[vertex].x -= (((int32)edge->v[vertex].x - edge->v[vertex ^ 1].x) * min_scale[vertex]) >> 8;
      clipped[vertex].y -= (((int32)edge->v[vertex].y - edge->v[vertex ^ 1].y) * min_scale[vertex]) >> 8;
      edge->clip_edge[vertex] = min_scale_index;
    }
    else {
      edge->clip_edge[vertex] = 0xFFFF;
    }
  }
  
  // If both of the points were outside, we need to check that the new clip is actually valid
  if(outside_pos[0] != 0 && outside_pos[1] != 0) {
    X3D_Vex2D_int16 mid = {(clipped[0].x + clipped[1].x) >> 1, (clipped[0].y + clipped[1].y) >> 1};
    
    for(line = 0; line < clip->total_pl; ++line) {
       if(((int32)clip->pl[line].normal.x * mid.x + (int32)clip->pl[line].normal.y * mid.y - clip->pl[line].d) < 0)
        return 0;
    }
  }
  
  edge->v[0] = clipped[0];
  edge->v[1] = clipped[1];
  
  return 1;
}

// Returns the list of point id's that are part of the given face
void get_face(int16* dest, int16* total_v, X3D_Prism2D* prism, int16 face) {
  int16 i;
  
  if(face == 0) {
    *total_v = prism->base_v;
    
    for(i = 0; i < prism->base_v; ++i)
      dest[i] = i;
  }
  else if(face == 1) {
    *total_v = prism->base_v;
    
    for(i = 0; i < prism->base_v; ++i)
      dest[i] = i + prism->base_v;
  }
  else {
    *total_v = 4;
    
    dest[0] = i - 2;
    dest[1] = i - 2 + 1;
    dest[2] = i - 2 + prism->base_v;
    dest[3] = i - 2 + 1 + prism->base_v;
  }
}

void clip_prism2d(X3D_Prism2D* prism, X3D_ClipRegion* r, X3D_RenderContext* context) {
  Edge e[prism->base_v * 3];
  uint16 i;
  
  for(i = 0; i < prism->base_v * 3; ++i) {
    int16 a, b;

    get_edge(prism, i, &a, &b);
    e[i].v[0] = prism->v[a];
    e[i].v[1] = prism->v[b];
    
    e[i].vis = clip_line(r, e + i);
    
    if(e[i].vis)
      x3d_draw_line_black(context, &e[i].v[0], &e[i].v[1]);
  }
  
}





















#if defined(__TIGCC__) || defined(WIN32)

typedef X3D_Prism X3D_Prism3D;

typedef struct X3D_TestContext {
  X3D_RenderContext context;
  X3D_EngineState state;
  X3D_RenderDevice device;
} X3D_TestContext;

void TEST_x3d_project_prism3d(X3D_Prism2D* dest, X3D_Prism3D* p, X3D_RenderContext* context) {
  uint16 i;
  for(i = 0; i < p->base_v * 2; ++i) {
    x3d_vex3d_int16_project(dest->v + i, p->v + i, context);
  }

  dest->base_v = p->base_v;
}

static void x3d_test_init_screen(X3D_TestContext* context) {
  x3d_enginestate_init(&context->state, 5, 1000);
  x3d_renderdevice_init(&context->device, 240, 128);

  x3d_rendercontext_init(&context->context, context->device.dbuf, LCD_WIDTH, LCD_HEIGHT, LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_60, 0);
}

static void x3d_test_copy_prism3d(X3D_Prism3D* dest, X3D_Prism3D* src) {
  dest->base_v = src->base_v;

  uint16 i;
  for(i = 0; i < src->base_v * 2; ++i) {
    dest->v[i] = src->v[i];
  }
}

// Stuff to make visual studio shut up
#ifdef WIN32
#define INT_HANDLER int
#define GetIntVec(...) 0
#define DUMMY_HANDLER 0
#define SetIntVec(...) ;
#define FontSetSys(...) ;
#define LCD_WIDTH 240
#define LCD_HEIGHT 128
#endif

void x3d_prism2d_clip(X3D_Prism2D* prism, X3D_ClipRegion* clip, X3D_RenderContext* context);
X3D_ClipRegion* x3d_construct_clipregion(X3D_Vex2D_int16* v, uint16 total_v);

void x3d_test() {
  FontSetSys(F_4x6);
  
  clrscr();

  // Redirect interrupt handlers
  INT_HANDLER old_int_1 = GetIntVec(AUTO_INT_1);
  INT_HANDLER old_int_5 = GetIntVec(AUTO_INT_5);

  SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
  SetIntVec(AUTO_INT_5, DUMMY_HANDLER);

  X3D_TestContext test;

  x3d_test_init_screen(&test);

  // Initialize the camera
  X3D_Camera* cam = &test.context.cam;
  cam->pos = (X3D_Vex3D_fp16x16){ 0, 0, 0 };
  cam->angle = (X3D_Vex3D_angle256){ 0, 0, 0 };

  // Allocate some prisms
  X3D_Prism* prism3d = malloc(sizeof(X3D_Prism3D) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  X3D_Prism* prism3d_temp = malloc(sizeof(X3D_Prism3D) + sizeof(X3D_Vex3D_int16) * 50 * 2);
  X3D_Prism2D* prism2d = malloc(sizeof(X3D_Prism2D) + sizeof(X3D_Vex2D_int16) * 50 * 2);

  // Construct the viewing frustum
  X3D_Frustum* frustum = malloc(sizeof(X3D_Frustum) + sizeof(X3D_Plane) * 20);

  x3d_frustum_from_rendercontext(frustum, &test.context);

  x3d_frustum_print(frustum);
  X3D_LOG_WAIT(&test.context, "");


  x3d_prism_construct(prism3d, 8, 25 * 3, 50 * 3, (X3D_Vex3D_uint8){ 0, 0, 0 });

  X3D_Vex2D_int16 clip[4] = {
    { 30, LCD_HEIGHT - 20 },
    { LCD_WIDTH - 20, 0 },
    { LCD_WIDTH - 20, LCD_HEIGHT - 70 },
    { 30, LCD_HEIGHT - 1 }
  };


  X3D_ClipRegion* r = x3d_construct_clipregion(clip, 4);
  clrscr();
  
  _Bool has_first = 0, has_second = 0;
  
  int16 cx = LCD_WIDTH / 2, cy = LCD_HEIGHT / 2;
  
  X3D_Vex2D first, second;
  int16 inside;
  
  Edge edge;
  
  goto test2;
  
  // Test the new line clipping algorithm
  do {
    clrscr();
    
    // Draw the clipping region
    uint16 i;
    for(i = 0; i < 4; i++) {
      x3d_draw_line_black(&test.context, clip + i, clip + ((i + 1) % 4));
    }
    
    // Draw the cursor
    DrawPix(cx, cy, A_NORMAL);
    
    if(has_first && has_second) {
      x3d_draw_line_black(&test.context, &first, &second);
      printf("clip_line reports %s\n", inside ? "INSIDE" : "OUTSIDE");
    }
    
    x3d_renderdevice_flip(&test.device);
    
    if(_keytest(RR_UP))
      --cy;
    else if(_keytest(RR_DOWN))
      ++cy;
    else if(_keytest(RR_LEFT))
      --cx;
    else if(_keytest(RR_RIGHT))
      ++cx;
    else if(_keytest(RR_ESC))
      goto done;
    else if(_keytest(RR_F1)) {
      has_first = 0;
      has_second = 0;
    }
    else if(_keytest(RR_ENTER)) {
      if(!has_first) {
        first.x = cx;
        first.y = cy;
        has_first = 1;
      }
      else if(!has_second) {
        second.x = cx;
        second.y = cy;
        has_second = 1;
        
        edge.v[0] = first;
        edge.v[1] = second;
        
        inside = clip_line(r, &edge);
        
        first = edge.v[0];
        second = edge.v[1];
        
        x3d_renderdevice_flip(&test.device);
        
      }
      
      while(_keytest(RR_ENTER)) ;
    }
  } while(1);
  
  
  
  
  
  
  
  
  
  
test2:
  //X3D_LOG_WAIT(&test.context, "Enter loop\n");

  do {
    x3d_mat3x3_fp0x16_construct(&cam->mat, &cam->angle);
    x3d_test_copy_prism3d(prism3d_temp, prism3d);

    prism3d_temp->draw_edges = 0xFFFFFFFF;

    // Move the prism relative to the camera
    X3D_Vex3D_int16 cam_pos = { cam->pos.x >> 15, cam->pos.y >> 15, cam->pos.z >> 15 };


    uint16 i;
    for(i = 0; i < prism3d_temp->base_v * 2; ++i) {
      prism3d_temp->v[i] = vex3d_int16_sub(prism3d_temp->v + i, &cam_pos);

      X3D_Vex3D_int16 temp;
      x3d_vex3d_int16_rotate(&temp, prism3d_temp->v + i, &cam->mat);

      prism3d_temp->v[i] = temp;
    }

    for(i = 0; i < prism3d->base_v * 2; ++i) {
      x3d_vex3d_int16_project(prism2d->v + i, prism3d_temp->v + i, &test.context);
    }

    prism2d->base_v = prism3d->base_v;

    clrscr();
    //x3d_prism_render(prism3d_temp, &test.context);

    //for(i = 0; i < 4; i++) {
    //  x3d_draw_line_black(&test.context, clip + i, clip + ((i + 1) % 4));
    //}

    //X3D_LOG_WAIT(&test.context, "Draw lines\n");

    //x3d_prism2d_clip(prism2d, r, &test.context);
    //clip_prism2d(prism2d, r, &test.context);

    x3d_draw_clipped_prism3d_wireframe(prism3d_temp, frustum);

    x3d_renderdevice_flip(&test.device);

    X3D_Vex3D_int32 dir = { (int32)cam->mat.data[2] * 4, (int32)cam->mat.data[5] * 4, (int32)cam->mat.data[8] * 4 };

    if(_keytest(RR_UP)) {
      cam->pos.x += dir.x;
      cam->pos.y += dir.y;
      cam->pos.z += dir.z;
    }
    else if(_keytest(RR_DOWN)) {
      cam->pos.x -= dir.x;
      cam->pos.y -= dir.y;
      cam->pos.z -= dir.z;
    }

    if(_keytest(RR_LEFT)) {
      cam->angle.y -= 1;
    }
    else if(_keytest(RR_RIGHT)) {
      cam->angle.y += 1;
    }
    if(_keytest(RR_ESC)) {
      break;
    }



  } while(1);


done:


  x3d_renderdevice_cleanup(&test.device);
  x3d_enginestate_cleanup(&test.state);

  SetIntVec(AUTO_INT_1, old_int_1);
  SetIntVec(AUTO_INT_5, old_int_5);

  
}

#endif