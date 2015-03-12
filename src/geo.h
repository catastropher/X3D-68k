// Header File
// Created 3/4/2015; 10:42:27 PM
#pragma once

#include "fix.h"
#include "math.h"

#define NORMAL_BITS 15		// The number of fractional bits used to represent a normal in fixed point
#define MAX_POINTS 20		// Mamimum number of points a polygon can have
#define MAX_PLANES 20		// Maximum number of planes a view frustum can have

#define ANG_90 64
#define ANG_180 128

#define VERTICAL_LINE_SLOPE 0x7FFF

#define DIST_TO_NEAR_PLANE 10


// The 8 verticies of a cube
enum {
	VEX_UBL,
	VEX_UTL,
	VEX_UTR,
	VEX_UBR,
	VEX_DBL,
	VEX_DTL,
	VEX_DTR,
	VEX_DBR
};

// The 6 planes/faces of a cube
enum {
	PLANE_BOTTOM,
	PLANE_TOP,
	PLANE_LEFT,
	PLANE_RIGHT,
	PLANE_BACK,
	PLANE_FRONT
};

// A 3D vertex or vector with short values
typedef struct Vex3D {
	short x, y, z;
} Vex3D;

// A special padded 3D vector for the output of a 3D rotation
typedef struct{
	short x;
	short pad1;
	short y;
	short pad2;
	short z;
	short pad3;
} Vex3D_rot;

// A 3D vertex or vector with char (small) values
typedef struct Vex3Ds {
	unsigned char x, y, z;
} Vex3Ds;

// A 2D vertex or vector
typedef struct {
	short x, y;
} Vex2D;

// A 3D plane, defined by the surface normal and the distance to the
// plane from the origin i.e. AX + BY + CZ + D = 0
typedef struct {
	Vex3D normal;
	short d;
} Plane;

// A 3D polygon with an arbitrary number of points (with an upper bound)
// TODO: rename to Polygon3D
typedef struct {
	Vex3D v[MAX_POINTS];
	short total_v;
} Polygon;

typedef Polygon Polygon3D;

// A 2D polygon with an arbitrary number of points
typedef struct {
	Vex2D v[MAX_POINTS];
	short total_v;
} Polygon2D;

// The "viewing pyramid", a region of visible space bounded by a number of planes
typedef struct {
	Plane p[MAX_PLANES];
	short total_p;
} Frustum;

// A 3x3 matrix
typedef short Mat3x3[3][3];

// A camera, described by its position, angle, rotation matrix,
// and direction
typedef struct {
	Vex3D pos;
	Vex3Ds angle;
	Mat3x3 mat;
	Vex3D dir;
	short dist_from_origin;		// Distance from the origin
} Camera;

// A rendering context, which describes the dimensions of the screen, the view
// frustum, and information about the camera
typedef struct RenderContext {
	short w, h;
	short x, y;
	short dist;
	short fov;
	short center_x, center_y;
	
	Frustum frustum;
	Frustum frustum_unrotated;
	
	Camera cam;
	
	unsigned char* screen;
} RenderContext;

// A cube (really a convex octahedron) that is the basic unit of levels
typedef struct {
	Vex3D v[8];
	Vex3D normal[8];
	short cube[6];
	
	unsigned short last_frame;
	unsigned short edge_bits;
} Cube;


extern const short sintab[256];
extern const int cube_vertex_tab[6][5];

// ==============================math.c==============================
short dot_product(Vex3D* a, Vex3D* b);
void cross_product(Vex3D* a, Vex3D* b, struct Vex3D* dest);
void project_vex3d(RenderContext* rc, Vex3D* src, Vex2D* dest);
void normalize_vex3d(Vex3D* v);
short asm_rotate_vex3d(Vex3D *src asm("a4"), Mat3x3* mat asm("a5"), Vex3D_rot *dest asm("a3"));
void rotate_vex3d(Vex3D* src, Mat3x3* mat, Vex3D* dest);
short get_vex3d_magnitude(Vex3D* v);

void construct_plane(Vex3D* a, Vex3D* b, Vex3D* c, Plane* dest);
void calculate_frustum_plane_normals(RenderContext* c);
void calculate_frustum_plane_distances(RenderContext* c);
void calculate_frustum_rotated_normals(RenderContext* c);

void sub_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest);
void add_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest);

void construct_mat3x3(Vex3Ds *angle, Mat3x3 *dest);
void mul_mat3x3(Mat3x3* a, Mat3x3* b, Mat3x3* res);

inline short sinfp(unsigned char angle) __attribute__((pure));
inline short cosfp(unsigned char angle) __attribute__((pure));
inline short tanfp(unsigned char angle) __attribute__((pure));

void construct_cube(short x, short y, short z, short posx, short posy, short posz, Vex3Ds* angle, Cube* c);

void project_polygon3d(Polygon3D* src, RenderContext* c, Polygon2D* dest);
void cube_get_face(Vex3D v[8], short face,  Vex3D dest[4]);

// ==============================clip.c==============================
char clip_polygon_to_plane(Polygon* poly, Plane* plane, Polygon* dest);
char clip_polygon_to_frustum(Polygon* src, Frustum* f, Polygon* dest);

// ==============================render.c==============================
void init_render_context(short w, short h, short x, short y, unsigned char fov, RenderContext* c);
void render_cube(Cube* c, RenderContext* context);
void set_cam_pos(RenderContext* c, short x, short y, short z);
void set_cam_angle(RenderContext* c, unsigned char x, unsigned char y, unsigned char z);
void draw_polygon(Polygon2D* p, RenderContext* context);
extern void draw_clip_line(register short asm("%d0"), register short asm("%d1"), register short asm("%d2"),
	register short asm("%d3"), register void* asm("%a2"));


// ==============================util.c==============================
void print_vex3d(Vex3D* v);
void print_plane(Plane* p);
void print_polygon(Polygon* p);
void print_frustum(Frustum* f);

// ==============================fastsqrt.c==============================
unsigned long fastsqrt(unsigned long x) __attribute__((pure));


