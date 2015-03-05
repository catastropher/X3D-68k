// Header File
// Created 3/4/2015; 10:42:27 PM
#pragma once

#include "fix.h"

#define NORMAL_BITS 15


// A 3D vertex or vector
typedef struct {
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

// Mamimum number of points a polygon can have
#define MAX_POINTS 20

// A polygon with an arbitrary number of points (with an upper bound)
typedef struct {
	short total_v;
	Vex3D v[MAX_POINTS];
} Polygon;

#include "math.h"