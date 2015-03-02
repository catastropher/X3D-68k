// C Source File
// Created 2/25/2015; 5:38:15 PM

#include <tigcclib.h>

#include "3D.h"
#include "graphics.h"
#include "clip.h"

#include "extgraph.h"


#define FRAC_BITS 8
#define EVAL_BITS 5
#define VERTICAL_LINE 0x7FFF

char signof(short val);

//char get_signof(short val) {
//	return (val < 0 ? -1 : val > 0);
//}

inline short eval_line(Line2D* line, short x) {
	return (((long)line->slope * x) >> FRAC_BITS) + line->b;
}

inline long eval_line_long(Line2D* line, long x) {
	return ((((long)line->slope * x) >> FRAC_BITS) >> EVAL_BITS) + line->b;
}

char point_valid_side(Line2D* line, Vex2D* point) {	
	char sign;
	
	if(line->slope == VERTICAL_LINE) {
		sign = signof(point->x - line->b);
	}
	else {
		sign = signof(point->y - eval_line(line, point->x));
		
		short dy = point->y - eval_line(line, point->x);
		
		if(abs(dy) < 2)
			return 0;
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
		dest->slope = ((long)dy << FRAC_BITS) / dx;
		dest->b = start->y - (((long)dest->slope * start->x) >> FRAC_BITS);
		dest->sign = signof(center->y - eval_line(dest, center->x));
		dest->start = min(start->x, end->x);
		dest->end = max(start->x, end->x);
	}
}

char line2d_intersect(Line2D* a, Line2D* b, Vex2D* res, Vex2D* center) {
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
		
		//printf("Slope a: %ld\n", a->slope);
		//printf("Slope b: %ld\n", b->slope);
		//ngetchx();
		
		//if(abs((long)a->slope - b->slope) > 32767) {
		//	error("Slope out of bounds");
		//}
		
		long x = (((long)b->b - a->b) << (FRAC_BITS + EVAL_BITS)) / ((long)a->slope - b->slope);
		res->y = eval_line_long(a, x);
		res->x = x >> EVAL_BITS;
		
		return 1;
		return res->x >= a->start && res->x <= a->end;
	}
	
	
	
}

void print_polygon2(Polygon* p) {
	int i;
	
	for(i = 0; i < p->total_points; i++) {
		printf("{%d,%d} s: %ld b: %d\n", p->p[i].v.x, p->p[i].v.y, p->line[i].slope, p->line[i].b);
	}
}

void polygon_clip_edge(Polygon* p, Line2D* edge, Polygon* dest, Vex2D* center) {
	int point;
	int next_point;
	char next;
	Vex2D clip_pos;
	short out[20];
	short out_pos = 0;
	int i;
	
	char side = point_valid_side(edge, &p->p[0].v);
	char next_side;
	char clipped;
	
	dest->total_points = 0;
	
	for(point = 0; point < p->total_points; point++) {
		Line2D* line = &p->line[point];
		
		next_point = (point + 1) % p->total_points;
		next_side = point_valid_side(edge, &p->p[next_point].v);
		
		if(side != -1) {
			dest->p[dest->total_points] = p->p[point];
			dest->line[dest->total_points++] = *line;
		}
		
		clipped = 0;
		if(side + next_side == 0 && side) {
			if(line2d_intersect(line, edge, &clip_pos, center)) {
				dest->p[dest->total_points] = (Point){1, clip_pos};
				dest->line[dest->total_points++] = p->line[point];
					
				//out[out_pos++] = dest->total_points++;
				clipped = 1;
			}
		}
		
		if(clipped || side == 0) {
			out[out_pos++] = dest->total_points - 1;
		}
		
		side = next_side;
	}
	
	//for(i = 0; i < out_pos; i++)
		//printf("Out: %d\n", out[i]);
	
	for(i = 0; i < out_pos - 1; i++) {
		if(out[i] == out[i + 1] - 1) {
			//printf("Case\n");
			dest->line[out[i]] = *edge;
			dest->line[out[i]].draw = 0;
		}
	}
	
	if(out_pos > 0 && out[0] == 0 && out[out_pos - 1] == dest->total_points - 1) {
		dest->line[dest->total_points - 1] = *edge;
		dest->line[dest->total_points - 1].draw = 0;
	}
	
	
	
	/*if(out_pos == 2) {
		short min_p = min(out[0], out[1]);
		short max_p = max(out[0], out[1]);
		
		//printf("Min out: %d\n", min_p);
		//printf("Max out: %d\n", max_p);
	
#if 1	
		if(min_p == 0 && max_p == dest->total_points - 1) {
			dest->line[dest->total_points - 1] = *edge;
			dest->line[dest->total_points - 1].draw = 0;
		}
		else {
			dest->line[min_p] = *edge;
			dest->line[min_p].draw = 0;
		}
#endif
	}*/
	
	//printf("Out_pos: %d\n", out_pos);
	
	
#if 0
	if(out_pos != 0 && out_pos != 2) {
		printf("Invalid out B\n");
		
		print_polygon2(p);
		printf("Edge: s: %ld b: %d\n", edge->slope, edge->b);
		ngetchx();
	}
#endif
		
		
		
		
	//	error("Invalid out B\n");
}

	
void print_polygon(Polygon* p);
	
 
Polygon* clip_polygon(Polygon* p, Polygon* clip, Polygon* temp_a, Polygon* temp_b) {
	Vex2D center = {0, 0};
	int i;
	Polygon *p1 = temp_a;
	Polygon *p2 = temp_b;
	
	// Clip against the first edge
	polygon_clip_edge(p, &clip->line[clip->total_points - 1], p2, &center);
	
	
	for(i = 0; i < clip->total_points - 1; i++) {
		//print_polygon(p2);
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
		p->p[i].was_clipped = 0;
		get_line_info(&p->line[i], &v[i], &v[next], &center); 
		p->line[i].draw = 1;
	}
}

void draw_polygon2(Polygon* p, void* screen) {	
	int i;
	int prev = p->total_points - 1;
	
	for(i = 0; i < p->total_points; i++) {
		if(p->line[prev].draw) {
			//FastLine_Draw_R(screen, p->p[prev].v.x, p->p[prev].v.y, p->p[i].v.x, p->p[i].v.y);
			draw_clip_line(p->p[prev].v.x, p->p[prev].v.y, p->p[i].v.x, p->p[i].v.y, screen);
		}
		
		prev = i;
	}
}

void draw_polygon(Polygon* p) {	
	int i;
	int prev = p->total_points - 1;
	
	for(i = 0; i < p->total_points; i++) {
		if(p->line[prev].draw) {
			draw_clip_line(p->p[prev].v.x, p->p[prev].v.y, p->p[i].v.x, p->p[i].v.y, LCD_MEM);
		}
		
		prev = i;
	}
	
	
	

#if 0
	for(i = 0; i < p->total_points - 1; i++) {
		draw_clip_line(p->p[i].v.x, p->p[i].v.y, p->p[i + 1].v.x, p->p[i + 1].v.y, LCD_MEM); 
	}
	
	draw_clip_line(p->p[0].v.x, p->p[0].v.y, p->p[p->total_points - 1].v.x, p->p[p->total_points - 1].v.y, LCD_MEM);
#endif
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

void draw_line(Line2D* line) {
	long i, d;
	
	for(i = 0; i < 240; i++) {
		for(d = 0; d < 128; d++) {
			if(abs(eval_line(line, i) - d) < 1)
				DrawPix(i, d, A_NORMAL);
		}
	}
}

void test_polygon_clipper2() {
	Vex2D points[] = {
		{
			101, 83
		},
		{
			25, 86
		},
		{
			3, 105
		},
		{
			118, 98
		}
	};
	
	Line2D line;
	
	Polygon p2;
	Vex2D center;
	
	line.slope = -15;
	line.b = 106;
	line.sign = -1;
	
	//line.slope = -256;
	//line.b = 30;
	
	clrscr();
	Polygon p;
	
	make_polygon(points, 4, &p);
	draw_polygon(&p);
	draw_line(&line);
	
	ngetchx();
	
	polygon_clip_edge(&p, &line, &p2, &center);
	
	ngetchx();
	
	clrscr();
	draw_polygon(&p2);
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
	
	
	int cx = 39, cy = 63;
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
				draw_clip_line(cx, cy, p[p_pos - 2].x, p[p_pos - 2].y, LCD_MEM);
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
		//draw_polygon(&clipp);
		ngetchx();
		
		clrscr();
		draw_polygon(&pp);
		draw_polygon(&clipp);
	} while(ngetchx() != KEY_ESC);
}