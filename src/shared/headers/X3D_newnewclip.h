typedef struct X3D_RasterEdge {
  uint8 flags;
  
  int16 min_y;

  int16 max_y;
  int16 max_x;

  int16* x_data;
  int16 min_x;
} X3D_RasterEdge;

enum {
  EDGE_HORIZONTAL = 1,
  EDGE_INVISIBLE = 2
};

typedef struct X3D_RenderStack {
  uint8* ptr;
  uint8* base;
  uint8* end;
} X3D_RenderStack;

typedef struct X3D_RasterRegion {
  int16 min_y;
  int16 max_y;
  
  int16* x_left;
  int16* x_right;
} X3D_RasterRegion;

_Bool intersect_rasterregion(X3D_RasterRegion* portal, X3D_RasterRegion* region);
void rasterize_rasterregion(X3D_RasterRegion* region, void* screen, uint16 color);
void* renderstack_save(X3D_RenderStack* stack);
void renderstack_restore(X3D_RenderStack* stack, void* ptr);
void x3d_init_clip_window(X3D_RenderStack* stack, X3D_Context* context, X3D_RasterRegion* region, Vex2D* v, uint16 total_v);
void renderstack_init(X3D_RenderStack* stack, uint16 size);
void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, int16 min_y, int16 max_y);
_Bool construct_and_clip_rasterregion(X3D_RenderStack* stack, X3D_RasterRegion* portal, X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, X3D_RasterRegion* dest);

