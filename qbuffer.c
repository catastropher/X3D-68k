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
	char start, end;
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
	char sign;
	
	if(line->slope == VERTICAL_LINE) {
		sign = signof(point->x - line->b);
	}
	else {
		sign = signof(point->y - eval_line(line, point->x));
	}
	
	if(sign == 0)
		return 0;
	
	return sign == line->sign ? 1 : -1;
}

inline void get_line_info(Line2D* dest, Vex2D* start, Vex2D* end, Vex2D* center) {
	short dx = end->x - start->x;
	short dy = end->y - start->y;
	
	if(dx == 0) {
		dest->slope = VERTICAL_LINE;
		dest->b = start->x;
		dest->sign = signof(center->x - start->x);
		dest->start = min(start->y, end->y);
		dest->end = max(start->y, end->y);
	}
	else {
		dest->slope = ((long)dy << 8) / dx;
		dest->b = start->y - (((long)dest->slope * start->x) >> 8);
		dest->sign = signof(center->y - eval_line(dest, center->x));
		dest->start = min(start->x, end->x);
		dest->end = max(start->x, end->x);
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
		
		return 1;
		return res->y >= a->start && res->y <= a->end;
	}
	else {
		res->x = (((long)b->b - a->b) << 8) / (a->slope - b->slope);
		res->y = eval_line(a, res->x);
		
		return 1;
		return res->x >= a->start && res->x <= a->end;
	}
	
	
	
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


void polygon_clip_edge(Polygon* p, Line2D* edge, Polygon* dest, Vex2D* center) {
	int point;
	int next_point;
	char next;
	Vex2D clip_pos;
	short out[20];
	short out_pos = 0;
	
	char side = point_valid_side(edge, &p->p[0].v);
	char next_side;
	
	printf("Points: %d\n", p->total_points);
	
	dest->total_points = 0;
	
	for(point = 0; point < p->total_points; point++) {
		Line2D* line = &p->line[point];
		
		next_point = (point + 1) % p->total_points;
		next_side = point_valid_side(edge, &p->p[next_point].v);
		
		if(side != -1) {
			dest->p[dest->total_points] = (Point){1, p->p[point].v};
			dest->line[dest->total_points++] = *line;
		}
		
		if(side + next_side == 0 && side) {
			if(line2d_intersect(line, edge, &clip_pos, center)) {
				dest->p[dest->total_points] = (Point){1, clip_pos};
				dest->line[dest->total_points] = p->line[point];
					
				out[out_pos++] = dest->total_points++;
			}
		}
		
		side = next_side;
	}
	
	if(out_pos == 2) {
		short min_p = min(out[0], out[1]);
		short max_p = max(out[0], out[1]);
		
		printf("Min out: %d\n", min_p);
		printf("Max out: %d\n", max_p);
	
#if 1	
		if(min_p == 0 && max_p == dest->total_points - 1) {
			dest->line[dest->total_points - 1] = *edge;
		}
		else {
			dest->line[min_p] = *edge;
		}
#endif
	}
	
	ngetchx();
}












 
#if 0
void polygon_clip_edge(Polygon* p, Line2D* edge, Polygon* dest, Vex2D* center) {
	int i;
	short prev = p->total_points - 1;
	Vex2D clip_pos;
	
	dest->total_points = 0;
	
	short out[20];
	short out_pos = 0;
	char last_side = point_valid_side(edge, &p->p[prev].v);
	Line2D first;
	char set_first = 0;
	char point_first = 1;
	
	if(last_side != -1) {
		dest->p[dest->total_points++] = p->p[prev];
		printf("Added point %d\n", prev);
		point_first = 1;
	}
	
	printf("Egde s: %ld, b: %d\n, sign: %d\n", edge->slope, edge->b, edge->sign);
	//ngetchx();
	
	for(i = 0; i < p->total_points; i++) {
		printf("Considering point %d\n", i);
		Line2D* line = &p->line[prev];
		char side = point_valid_side(edge, &p->p[i].v);
		
		//printf("Slope: %ld, b: %d\n, sign: %d\n", line->slope, line->b, line->sign);
		//ngetchx();
		
		if(side + last_side == 0 && side) {
			//printf("Outside\n");
			
			printf("We need to clip it\n");
			if(line2d_intersect(line, edge, &clip_pos, center)) {
				// Add the new clipped point
				out[out_pos++] = dest->total_points;
				
				dest->p[dest->total_points] = (Point){1, clip_pos};
				
				
				if(i == 0) {
					first = *line;
					set_first = 1;
					printf("Set first line case A\n");
				}
				else
					dest->line[dest->total_points - 1] = *line;
					
				dest->total_points++;
				
				printf("Line clipped and point added\n");
			}
			else {
				printf("...but they don't intersect\n");
			}
		}
		
		if(i != p->total_points - 1 && side != -1) {
			dest->p[dest->total_points] = p->p[i];
			
			if(i == 0 && !set_first) {
				first = *line;
				printf("Set first line case B\n");
				printf("First slope: %ld\n", first.slope);
			}
			else
				dest->line[dest->total_points - 1] = *line;
			
			dest->total_points++;
			
			printf("Point added\n");
		}
		
		prev = i;
		last_side = side;
		
		ngetchx();
	}
	
	if(dest->total_points != 0) {
		if(!point_first)
			dest->line[dest->total_points - 1] = first;
		else
			dest->line[0] = first;
	}
	
	
	if(out_pos != 0) {
		
		short min_p = min(out[0], out[1]);
		short max_p = max(out[0], out[1]);
		
		printf("Min out: %d\n", min_p);
		printf("Max out: %d\n", max_p);
		
		if(min_p == 0 && max_p == dest->total_points - 1) {
			dest->line[dest->total_points - 1] = *edge;
		}
		else {
			dest->line[min_p] = *edge;
		}
	}
	
	
	//ngetchx();
}

#endif
	
void print_polygon(Polygon* p);
	
 
Polygon* clip_polygon(Polygon* p, Polygon* clip, Polygon* temp_a, Polygon* temp_b)
{
	Vex2D center = {0, 0};
	int i;
	Polygon *p1 = temp_a;
	Polygon *p2 = temp_b;
	
	// Clip against the first edge
	polygon_clip_edge(p, &clip->line[p->total_points - 1], p2, &center);
	
	
	for(i = 0; i < clip->total_points - 1; i++) {
		print_polygon(p2);
		SWAP(p1, p2);
		
		if(p1->total_points == 0) {
			p2->total_points = 0;
			break;
		}
		
		polygon_clip_edge(p1, &clip->line[i], p2, &center);
	}
	
	return p2;
}
	

void make_polygon(Vex2D* v, int points, Polygon* p) {
	p->total_points = points;
	int i;
	Vex2D center = {0, 0};
	
	for(i = 0; i < points; i++) {
		center.x += v[i].x;
		center.y += v[i].y;
	}
	
	center.x /= points;
	center.y /= points;
	
	for(i = 0; i < points; i++) {
		short next = (i + 1) % points;
		p->p[i].v = v[i];
		get_line_info(&p->line[i], &v[i], &v[next], &center); 
	}
}

void draw_polygon(Polygon* p) {	
	int i;
	for(i = 0; i < p->total_points - 1; i++) {
		draw_clip_line(p->p[i].v.x, p->p[i].v.y, p->p[i + 1].v.x, p->p[i + 1].v.y, LCD_MEM); 
	}
	
	draw_clip_line(p->p[0].v.x, p->p[0].v.y, p->p[p->total_points - 1].v.x, p->p[p->total_points - 1].v.y, LCD_MEM);
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

void print_polygon(Polygon* p) {
	int i;
	
	printf("===========\n");
	
	for(i = 0; i < p->total_points; i++) {
		Line2D* line = &p->line[i];
		printf("s: %ld, i: %d, s: %d\n", line->slope, line->b, line->sign); 
	}
	
	printf("===========\n");
	ngetchx();
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
	
	short p_pos = 0;
	
	
	int cx = 39, cy = 64;
	unsigned short key;
	
	draw_polygon(&clipp);
	
	do {
		key = ngetchx();
		
		DrawPix(cx, cy, A_REVERSE);
		
		if(key == KEY_UP)
			cy -= 3;
		else if(key == KEY_DOWN)
			cy += 3;
		else if(key == KEY_LEFT)
			cx -= 3;
		else if(key == KEY_RIGHT)
			cx += 3;
		else if(key == KEY_ENTER) {
			p[p_pos++] = (Vex2D){cx, cy};
			
			if(p_pos != 1)
				DrawLine(cx, cy, p[p_pos - 2].x, p[p_pos - 2].y, A_NORMAL);
		}
		
		DrawPix(cx, cy, A_NORMAL);
	} while(key != KEY_ESC);
	
	make_polygon(p, p_pos, &pp);
	
	//for(i = 0; i < p_pos; i++) {
	//	Line2D* line = &pp.line[i];
//		printf("Slope: %ld, b: %d, sign: %d\n", line->slope, line->b, line->sign);
//	}

#if 0
	for(i = 0; i < clipp.total_points; i++) {
		Line2D* line = &clipp.line[i];
		printf("Slope: %ld, b: %d, sign: %d\n", line->slope, line->b, line->sign);
	}
	
	ngetchx();
#endif
	
	
	draw_polygon(&pp);
	ngetchx();
	draw_polygon(&clipp);
	ngetchx();
	
	Polygon* res = clip_polygon(&pp, &clipp, &temp_a, &temp_b);
	
	clrscr();
	draw_polygon(&clipp);
	draw_polygon(res);
	ngetchx();
	
	do {
		clrscr();
		draw_polygon(res);
		draw_polygon(&clipp);
		ngetchx();
		
		clrscr();
		draw_polygon(&pp);
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