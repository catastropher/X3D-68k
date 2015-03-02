// Header File
// Created 3/1/2015; 9:46:21 AM
#pragma once

struct CBuffer;

#include "3D.h"
#include "sbuffer.h"

typedef struct {
	long slope;
	short b;
	char sign;
	char start, end;
	char draw;
} Line2D;

#define MAX_POINTS 20

typedef struct {
	char was_clipped;
	Vex2D v;
} Point;

typedef struct {
	Point p[MAX_POINTS];
	short total_points;
	Line2D line[MAX_POINTS];
} Polygon;

Polygon* clip_polygon(Polygon* p, Polygon* clip, Polygon* temp_a, Polygon* temp_b);
inline void get_line_info(Line2D* dest, Vex2D* start, Vex2D* end, Vex2D* center);
void draw_polygon(Polygon* p);
void make_polygon(Vex2D* v, int points, Polygon* p);
void draw_polygon2(Polygon* p, void* screen);