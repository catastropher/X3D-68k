#include "X3D_common.h"
#include "render/X3D_polyvertex.h"
#include "X3D_clip.h"



void x3d_polyvertex_make(X3D_PolyVertex* v, X3D_Vex2D v2d, int32 uz, int32 vz, int32 zz, int32 i) {
  v->v2d = v2d;
  v->u = uz;
  v->v = vz;
  v->z = zz;
  v->intensity = i;
}

void x3d_polyline2_add_vertex(X3D_PolyLine2* line, X3D_PolyVertex v) {
  line->v[line->total_v++] = v;
}

enum {
  A_IN = 1,
  B_IN = 2
};

enum {
  A_IN_B_IN   = A_IN | B_IN,
  A_IN_B_OUT  = A_IN,
  A_OUT_B_IN  = B_IN,
  A_OUT_B_OUT = 0
};

typedef struct X3D_ClipperEdge {
  X3D_PolyVertex* a;
  X3D_PolyVertex* b;
} X3D_ClipperEdge;

uint16 x3d_clipper_edge_statecode(X3D_ClipperEdge edge, X3D_RasterRegion* r) {
  uint16 code = 0;
  
  if(x3d_rasterregion_point_inside(r, edge.a->v2d))
    code |= A_IN;
  
  if(x3d_rasterregion_point_inside(r, edge.b->v2d))
    code |= B_IN;
  
  return code;
}

void x3d_rasterregion_bin_search(X3D_Vex2D in, X3D_Vex2D out, X3D_Vex2D* res, X3D_RasterRegion* region);

void x3d_rasterregion_find_edge_intersection(X3D_RasterRegion* region, X3D_ClipperEdge edge, X3D_Vex2D* inter, _Bool* left) {
  x3d_rasterregion_bin_search(edge.a->v2d, edge.b->v2d, inter, region);
  
  X3D_Span* span = x3d_rasterregion_get_span(region, inter->y);
  
  *left = abs(span->left.x - inter->x) < abs(span->right.x - inter->x);
}




void x3d_polyline_clip_to_rasterregion(X3D_PolyLine* line, X3D_RasterRegion* region, X3D_PolyLine2* dest) {
  dest->total_v = 0;
  
  x3d_polyline2_add_vertex(dest, *line->v[0]);
  
  uint16 i;
  for(i = 1; line->total_v; ++i) {
    X3D_ClipperEdge edge = { .a = line->v[i - 1], .b = line->v[i] };
    
    switch(x3d_clipper_edge_statecode(edge, region)) {
      case A_IN_B_IN:
        x3d_polyline2_add_vertex(dest, *edge.b);
        break;
        
      case A_IN_B_OUT:
        break;
        
      case A_OUT_B_IN:
        break;
        
      case A_OUT_B_OUT:
        break;
    }
  }
}

