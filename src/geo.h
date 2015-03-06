// Header File
// Created 3/4/2015; 10:42:27 PM
#pragma once

#include "fix.h"
#include "math.h"

#define NORMAL_BITS 15		// The number of fractional bits used to represent a normal in fixed point
#define MAX_POINTS 20		// Mamimum number of points a polygon can have
#define MAX_PLANES 20		// Maximum number of planes a view frustum can have


// A 3D vertex or vector
typedef struct Vex3D {
	short x, y, z;
} Vex3D;

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