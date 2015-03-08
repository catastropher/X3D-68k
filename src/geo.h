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


// A 3D vertex or vector with short values
typedef struct Vex3D {
	short x, y, z;
} Vex3D;

// A 3D vertex or vector with char (small) values
typedef struct Vex3Ds {
	unsigned char x, y, z;
} Vex3Ds;

// A 2D vertex or vector
typedef struct {
	short x, y;
} Vex2D;

// A 3D plane, defined by the surface normal and the distance to the
// plane from the origin i.e. AX + BY + CZ - D = 0
typedef struct {
	Vex3D normal;
	short d;
} Plane;

// A polygon with an arbitrary number of points (with an upper bound)
typedef struct {
	Vex3D v[MAX_POINTS];
	short total_v;
} Polygon;

// The "viewing pyramid", a region of visible space bounded by a number of planes
typedef struct {
	Plane p[MAX_PLANES];
	short total_p;
} Frustum;

// A rendering context
typedef struct RenderContext {
	short w, h;
	short x, y;
	short scale;
	short fov;
	short center_x, center_y;
	
	
	Frustum frustum;
} RenderContext;

// A 3x3 matrix
typedef short Mat3x3[3][3];


extern const short sintab[256];



// ==============================math.c==============================
short dot_product(Vex3D* a, Vex3D* b);
void cross_product(Vex3D* a, Vex3D* b, struct Vex3D* dest);
void project_vex3d(RenderContext* rc, Vex3D* src, Vex2D* dest);
void construct_plane(Vex3D* a, Vex3D* b, Vex3D* c, Plane* dest);

void sub_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest);

void construct_mat3x3(Vex3Ds *angle, Mat3x3 *dest);
void mul_mat3x3(Mat3x3* a, Mat3x3* b, Mat3x3* res);

inline short sinfp(unsigned char angle);// __attribute__((pure));
inline short cosfp(unsigned char angle);// __attribute__((pure));
inline short tanfp(unsigned char angle) __attribute__((pure));

// ==============================clip.c==============================
char clip_polygon_to_plane(Polygon* poly, Plane* plane, Polygon* dest);
void print_polygon(Polygon* p);

// ==============================render.c==============================
void init_render_context(short w, short h, short x, short y, unsigned char fov, RenderContext* c);

