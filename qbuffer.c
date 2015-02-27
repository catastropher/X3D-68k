// C Source File
// Created 2/25/2015; 5:38:15 PM

#include <tigcclib.h>

#include "3D.h"
#include "graphics.h"

#define VERTICAL_LINE 0x7FFF

typedef struct {
	short x, y;
	short w, h;
} BoundBox;

typedef struct {
	long slope;
	short b;
	char sign;
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

char signof(short val);

//char get_signof(short val) {
//	return (val < 0 ? -1 : val > 0);
//}

inline short eval_line(Line2D* line, short x) {
	return (((long)line->slope * x) >> 8) + line->b;
}

char point_valid_side(Line2D* line, Vex2D* point) {
	if(line->slope == VERTICAL_LINE) {
		return signof(point->x - line->b) == line->sign;
	}
	else {
		return signof(point->y - eval_line(line, point->x)) == line->sign;
	}
}

inline void get_line_info(Line2D* dest, Vex2D* start, Vex2D* end, Vex2D* center) {
	short dx = end->x - start->x;
	short dy = end->y - start->y;
	
	if(dx == 0) {
		dest->slope = VERTICAL_LINE;
		dest->b = start->x;
		dest->sign = signof(center->x - start->x);
	}
	else {
		dest->slope = ((long)dy << 8) / dx;
		dest->b = start->y - (((long)dest->slope * start->x) >> 8);
		dest->sign = signof(center->y - eval_line(dest, center->x));
	}
}



//=========================================================================================
inline double dot(Vex2D* a, Vex2D* b)
{
	return a->x * b->x + a->y * b->y;
}
 
inline long cross(Vex2D* a, Vex2D* b)
{
	return (long)a->x * b->y - (long)a->y * b->x;
}
 
inline void vsub(Vex2D* a, Vex2D* b, Vex2D* res)
{
	res->x = a->x - b->x;
	res->y = a->y - b->y;
}
 
/* tells if vec c lies on the left side of directed edge a->b
 * 1 if left, -1 if right, 0 if colinear
 */
int left_of(Vex2D* a, Vex2D* b, Vex2D* c)
{
	Vex2D tmp1, tmp2;
	long x;
	vsub(b, a, &tmp1);
	vsub(c, b, &tmp2);
	x = cross(&tmp1, &tmp2);
	
	return x < 0 ? -1 : x > 0;
}
 
char line2d_intersect(Line2D* a, Line2D* b, Vex2D* res, Vex2D* center)
{
	//Line2D a;
	//Line2D b;
	
	//get_line_info(a, x0, x1, center);
	//get_line_info(b, y0, y1, center);
	
	if(a->slope == b->slope)
		return 0;
	else if(b->slope == VERTICAL_LINE) {
		SWAP(a, b);
	}
	
	if(a->slope == VERTICAL_LINE) {
		res->x = a->b;
		res->y = eval_line(b, a->b);
	}
	else {
		res->x = (((long)b->b - a->b) << 8) / (a->slope - b->slope);
		res->y = eval_line(a, res->x); 
	}
	
	return 1;
	
	
	
#if 0
	Vex2D dx, dy, d;
	vsub(x1, x0, &dx);
	vsub(y1, y0, &dy);
	vsub(x0, y0, &d);
	/* x0 + a dx = y0 + b dy ->
	   x0 X dx = y0 X dx + b dy X dx ->
	   b = (x0 - y0) X dx / (dy X dx) */
	int shift = 25;
	long dyx = cross(&dy, &dx);
	if (!dyx) return 0;
	//dyx = (((long)1 << shift) * cross(&d, &dx)) / dyx;
	
	long top = ((long)1 << shift) / dyx;
	
	top >>= (shift - 16 + 1);
	
	if(top > 32768)
		error("wrong");
	
	dyx = top * cross(&d, &dx);
	
	
	
	if (dyx <= 0 || dyx >= ((long)32768)) return 0;
 
 	long dyx_small = dyx;//(short)dyx;
 
	res->x = y0->x + (((long)dyx_small * dy.x) >> 15);
	res->y = y0->y + (((long)dyx_small * dy.y) >> 15);
	return 1;
#endif
}
 
/* === polygon stuff === */
typedef struct {
	int len, alloc;
	Vex2D v[MAX_POINTS];
} poly_t, *poly;

typedef poly_t Poly;
 
poly poly_new()
{
	return (poly)calloc(1, sizeof(poly_t));
}
 
void poly_free(poly p)
{
	//free(p->v);
	free(p);
}
 
void poly_append(poly p, Vex2D* v)
{
	/*if (p->len >= p->alloc) {
		p->alloc *= 2;
		if (!p->alloc) p->alloc = 4;
		p->v = (Vex2D* )realloc(p->v, sizeof(Vex2D) * p->alloc);
	}*/
	
	p->v[p->len++] = *v;
}
 
/* this works only if all of the following are true:
 *   1. poly has no colinear edges;
 *   2. poly has no duplicate vertices;
 *   3. poly has at least three vertices;
 *   4. poly is convex (implying 3).
*/
int poly_winding(poly p)
{
	return left_of(p->v, p->v + 1, p->v + 2);
}
 
#if 0
void poly_edge_clip(Poly* sub, Vex2D* x0, Vex2D* x1, int left, Poly* res, Vex2D* center)
{
	int i, side0, side1;
	Vex2D tmp;
	Vex2D* v0 = sub->v + sub->len - 1, *v1;
	res->len = 0;
 
	side0 = left_of(x0, x1, v0);
	if (side0 != -left) poly_append(res, v0);
 
	for (i = 0; i < sub->len; i++) {
		v1 = sub->v + i;
		side1 = left_of(x0, x1, v1);
		if (side0 + side1 == 0 && side0)
			/* last point and current straddle the edge */
			if (line_sect(x0, x1, v0, v1, &tmp, center))
				poly_append(res, &tmp);
		if (i == sub->len - 1) break;
		if (side1 != -left) poly_append(res, v1);
		v0 = v1;
		side0 = side1;
	}
}
#endif

void polygon_clip_edge(Polygon* p, Line2D* edge, Polygon* dest, Vex2D* center) {
	int i;
	Vex2D clip_pos;
	
	dest->total_points = 0;
	
	for(i = 1; i < p->total_points; i++) {
		Line2D* line = &p->line[i - 1];
		
		if(!point_valid_side(edge, &p->p[i].v)) {
			if(line2d_intersect(line, edge, &clip_pos, center)) {
				// Add the new clipped point
				dest->p[dest->total_points] = (Point){1, clip_pos};
				// FIXME: dest->line[dest->total_points++] = edge;
			}
		}
		else {
			dest->p[dest->total_points] = p->p[i];
			dest->line[dest->total_points++] = *line;
		}
	}
}
	
	
	
	
#if 0
	side0 = left_of(x0, x1, v0);
	if (side0 != -left) poly_append(res, v0);
 
	for (i = 0; i < sub->len; i++) {
		v1 = sub->v + i;
		side1 = left_of(x0, x1, v1);
		if (side0 + side1 == 0 && side0)
			/* last point and current straddle the edge */
			if (line_sect(x0, x1, v0, v1, &tmp, center))
				poly_append(res, &tmp);
		if (i == sub->len - 1) break;
		if (side1 != -left) poly_append(res, v1);
		v0 = v1;
		side0 = side1;
	}
}
#endif

 
Polygon* clip_polygon(Polygon* p, Polygon* clip, Polygon* temp_a, Polygon* temp_b)
{
	Vex2D center = {0, 0};
	int i;
	Polygon *p1 = temp_a;
	Polygon *p2 = temp_b;
	
	for(i = 0; i < clip->total_points; i++) {
		center.x += clip->p[i].v.x;
		center.y += clip->p[i].v.y;
	}
	
	center.x /= clip->total_points;
	center.y /= clip->total_points;
	
	// Clip against the first edge
	polygon_clip_edge(p, &p->line[clip->total_points - 1], p2, &center);
	
	for(i = 0; i < clip->total_points - 1; i++) {
		SWAP(p1, p2);
		
		if(p1->total_points == 0) {
			p2->total_points = 0;
			break;
		}
		
		polygon_clip_edge(p1, &p->line[i], p2, &center);
	}
	
	return p2;
}
	
# if 0
	poly p1 = temp_a, p2 = temp_b, tmp;
 
	int dir = poly_winding(clip);
	poly_edge_clip(sub, clip->v + clip->len - 1, clip->v, dir, p2, &center);
	for (i = 0; i < clip->len - 1; i++) {
		tmp = p2; p2 = p1; p1 = tmp;
		if(p1->len == 0) {
			p2->len = 0;
			break;
		}
		poly_edge_clip(p1, clip->v + i, clip->v + i + 1, dir, p2, &center);
	}
 
	return p2;
}
#endif
//==============================================================================================


void make_polygon(Vex2D* v, int points, Polygon* p) {
	p->total_points = points;
	int i;
	
	for(i = 0; i < points; i++) {
		p->p[i].v = v[i];
	}
}

void draw_polygon(Polygon* p) {	
	int i;
	for(i = 0; i < p->total_points - 1; i++) {
		DrawLine(p->p[i].v.x, p->p[i].v.y, p->p[i + 1].v.x, p->p[i + 1].v.y, A_NORMAL); 
	}
	
	DrawLine(p->p[0].v.x, p->p[0].v.y, p->p[p->total_points - 1].v.x, p->p[p->total_points - 1].v.y, A_NORMAL);
}

/*
poly clip_points(Poly* clip, Vex2D* v, int points) {
	poly_t p;
	
	make_polygon(v, points, &p);
	return clip_polygon(clip, &p, );
}
*/


Vex2D rand_point() {
	return (Vex2D){rand() % 240, rand() % 128};
}

void test_polygon_clipper() {
	clrscr();
	
	Vex2D p[8];
	
	#if 0
	do {
		Vex2D a1 = rand_point();
		Vex2D a2 = rand_point();
		Vex2D b1 = rand_point();
		Vex2D b2 = b1;//rand_point();
		
		b2.y = (b2.y + 30) % 128;
		
		clrscr();
		DrawLine(a1.x, a1.y, a2.x, a2.y, A_NORMAL);
		DrawLine(b1.x, b1.y, b2.x, b2.y, A_NORMAL);
		ngetchx();
		
		Vex2D res;
		Vex2D center;
		
		clrscr();
		
		if(line_sect(&a1, &a2, &b1, &b2, &res, &center)) {
			DrawStr(0, 0, "I", A_NORMAL);
		}
		else {
			DrawStr(0, 0, "N", A_NORMAL);
		}
		
		if(res.x >= 0 && res.x < LCD_WIDTH && res.y >= 0 && res.y < LCD_HEIGHT) {
			DrawPix(res.x, res.y, A_NORMAL);
		}
		
	} while(ngetchx() != KEY_ESC);
	#endif
	
	
	int i;
	for(i = 0; i < 4; i++) {
		p[i].x = rand() % 240;
		p[i].y = rand() % 128;
	}
	
	
	Vex2D clip[] = {
		{
			30, 30
		},
		{
			240 - 30, 30
		},
		{
			240 - 30, 128 - 30
		},
		{
			30, 128 - 30
		}
	};
	
	Polygon temp_a, temp_b;
	
	Polygon pp;
	make_polygon(p, 4, &pp);
	
	Polygon clipp;
	make_polygon(clip, 4, &clipp);
	
	draw_polygon(&pp);
	ngetchx();
	draw_polygon(&clipp);
	ngetchx();
	
	Polygon* res = clip_polygon(&clipp, &pp, &temp_a, &temp_b);
	
	clrscr();
	draw_polygon(&clipp);
	draw_polygon(res);
	ngetchx();
	
	do {
		clrscr();
		draw_polygon(res);
		ngetchx();
		
		clrscr();
		draw_polygon(&pp);
		ngetchx();
		draw_polygon(&clipp);
	} while(ngetchx() != KEY_ESC);
}






























#if 0
inline void calc_line_info(Line2D* line, Vex2D *a, Vex2D *b, Vex2D* center) {
	
}

// Subtracts two 2D vectors
inline void sub_vex2d(Vex2D* a, Vex2D* b, Vex2D* res) {
	res->x = a->x - b->x;
	res->y = a->y - b->y;
}

// Returns the "z" component of taking the cross product between
// two 2D vectors.
inline short cross(Vex2D* a, Vex2D* b) {
	return (short)((long)a->x * b->y - (long)a->y * b->x);
}

// Returns whether two lines intersect. If they do, this
//char line_intersect(Vex2D* x0, Vex2D* x1, Vex2D* y0, Vex2D* y1, Vex2D* res) {
	Vex2D dx, dy, d;
	
	sub_vex2d(x1, x0, &dx);
	sub_vex2d(y1, y0, &dy);
	sub_vex2d(x0, y0, &d);
	
	
	
	/* x0 + a dx = y0 + b dy ->
	   x0 X dx = y0 X dx + b dy X dx ->
	   b = (x0 - y0) X dx / (dy X dx) */
	double dyx = cross(&dy, &dx);
	if (!dyx) return 0;
	dyx = cross(&d, &dx) / dyx;
	if (dyx <= 0 || dyx >= 1) return 0;
 
	res->x = y0->x + dyx * dy.x;
	res->y = y0->y + dyx * dy.y;
	return 1;
}


// Clips all the points in a polygon against a line i.e. all the points have
// to be on the "correct" side of the line
void clip_polygon_edge(Polygon* poly, Line2D* line, Polygon* dest) {
	dest->total_points = 0;
	
	int i;
	for(i = 0; i < poly->total_points; i++) {
		//if(
	}
}


// Clips a polygon against a convex polygon
// Note: if the input polygon is convex, the output polygon is guaranteed
// to be convex
void clip_polygon(Polygon* clip, Polygon* poly, Polygon* dest) {
	dest->total_points = 0;
	
	// Clip poly's points against each bounding line of clip
	int i;
	for(i = 0; i < clip->total_points; i++) {
		
	}
}

#endif