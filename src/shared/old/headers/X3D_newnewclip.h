typedef struct X3D_Range {
  int16 min;
  int16 max;
} X3D_Range;

#define SCREEN_X_RANGE get_range(0, LCD_WIDTH - 1)
#define SCREEN_Y_RANGE get_range(0, LCD_HEIGHT - 1)


static inline X3D_Range get_range(int16 a, int16 b) {
  return (X3D_Range) { min(a, b), max(a, b) };
}

static inline _Bool range_overlap(X3D_Range a, X3D_Range b) {
  return !(a.max < b.min || a.min > b.max);
}

static inline _Bool in_range(X3D_Range r, int16 val) {
  return val >= r.min && val <= r.max;
}


typedef struct X3D_RasterEdge {  
  // int16 min_y;
  // int16 min_x;
  // int16 max_y;
  // int16 max_x;
  // int16 start_y;
  
  int16 start_x;
  int16 end_x;
  
  uint8 flags;
  
  int16* x_data;
  
  X3D_Range x_range;
  X3D_Range y_range;
  
} X3D_RasterEdge;

enum {
  EDGE_HORIZONTAL = 1,
  EDGE_INVISIBLE = 2,
  EDGE_V_SWAPPED = 4,
  EDGE_NEAR_CLIPPED = 8
};

typedef struct X3D_RenderStack {
  uint8* ptr;
  uint8* base;
  uint8* end;
} X3D_RenderStack;

typedef struct X3D_RasterRegion {
  // int16 min_y;
  // int16 max_y;
  
  X3D_Range y_range;
  
  int16* x_left;
  int16* x_right;
} X3D_RasterRegion;

typedef struct X3D_PolyLine {
  int16 total_e;
  int16* edges;
} X3D_PolyLine;



void generate_rasteredge(X3D_RenderStack* stack, X3D_RasterEdge* edge, Vex2D a, Vex2D b, X3D_Range region_y_range);


_Bool intersect_rasterregion(X3D_RasterRegion* portal, X3D_RasterRegion* region);
void rasterize_rasterregion(X3D_RasterRegion* region, void* screen, uint16 color);
void* renderstack_save(X3D_RenderStack* stack);
void renderstack_restore(X3D_RenderStack* stack, void* ptr);
void x3d_init_clip_window(X3D_RenderStack* stack, X3D_Context* context, X3D_RasterRegion* region, Vex2D* v, uint16 total_v);
void renderstack_init(X3D_RenderStack* stack, uint16 size);
_Bool construct_and_clip_rasterregion(X3D_RenderStack* stack, X3D_RasterRegion* portal, X3D_RasterEdge* raster_edge, uint16* edge_index, uint16 total_e, X3D_RasterRegion* dest);

