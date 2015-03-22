// C Source File
// Created 3/5/2015; 8:39:09 AM

#include "geo.h"
#include "math.h"
#include "console.h"
#include "error.h"

#include <tigcclib.h>

extern short cube_id;

// Clips a polygon against a plane. Returns whether a valid polygon remains.
// TODO: keep track of which edges have been clipped so we know which
// part of the polygon still needs to be drawn
char clip_polygon_to_plane(Polygon* poly, Plane* plane, Polygon* dest) {
	short i;
	short next_point;
	short in, next_in;
	short dot, next_dot;
	long t;
	
	short out[10];
	short out_pos = 0;
	
	dot = dot_product(&poly->v[0], &plane->normal);
	in = (dot >= plane->d);
	
	ADDR(poly);
	ADDR(plane);
	ADDR(dest);
	
	//printf("Dot: %d\nD: %d\n", dot, plane->d);
	//print_vex3d(&plane->normal);
	//print_vex3d(&poly->v[0]);
	//ngetchx();
	
	short total_outside = !in;
	
	dest->total_v = 0;
	
	for(i = 0; i < poly->total_v; i++) {
		next_point = (i + 1) % poly->total_v;
		
		// The vertex is inside the plane, so don't clip it
		if(in) {
			dest->v[dest->total_v] = poly->v[i];
			dest->draw[dest->total_v++] = poly->draw[i];
		}
			
		//errorif(!in, "Point not in!");
			
			
		next_dot = dot_product(&poly->v[next_point], &plane->normal);
		next_in = (next_dot >= plane->d);
		
		total_outside += !next_in;
		
	//	printf("Next dot: %d\n", next_dot);
		
		// The points are on opposite sides of the plane, so clip it
		if(in != next_in) {
			// Scale factor to get the point on the plane
			errorif((long)next_dot - dot == 0, "Clip div by 0");
			
			t = FIXDIV8(plane->d - dot, next_dot - dot);
			
			errorif(abs((long)plane->d - dot) > 32767, "plane->d too big");
			errorif(abs((long)next_dot - dot) > 32767, "next_dot too big");
			
			errorif(abs(t) > 32767, "Invalid clip t");
			//errorif(t == 0, "t == 0");
			
			if(t == 0) {
				//printf("T == 0\n");
			}
			
			if(cube_id == 12) {
				//printf("T: %ld\n", t);
			}
			
			//printf("Dist: %d\n", dot + plane->d);
			//printf("T: %ld Z: %d\n", t, poly->v[i].z);
			
			dest->v[dest->total_v].x = poly->v[i].x + FIXMUL8(((long)poly->v[next_point].x - poly->v[i].x), t);
			dest->v[dest->total_v].y = poly->v[i].y + FIXMUL8(((long)poly->v[next_point].y - poly->v[i].y), t);
			dest->v[dest->total_v].z = poly->v[i].z + FIXMUL8(((long)poly->v[next_point].z - poly->v[i].z), t);
			
			
			
			
			
			
			// Use floats to make sure we're not overflowing
			
			
			
			
			
			dest->draw[dest->total_v] = poly->draw[i];
			
			//printf("Dest z: %d\n", dest->v[dest->total_v].z);
			//printf("Should be: %d\n", -plane->d);
			
			//errorif(dest->v[dest->total_v].z < DIST_TO_NEAR_PLANE / 2, "Invalid clip: %d", dest->v[dest->total_v].z);
			
			dest->total_v++;
		}
		
		if(next_in != in) {
			out[out_pos++] = dest->total_v - 1;
		}
		
		dot = next_dot;
		in = next_in;
	}
	
	//printf("total outside: %d\ntotal: %d", total_outside, dest->total_v);
	
	if(dest->total_v > 4) {
		//clrscr();
		
		int i;
		
		for(i = 0; i < dest->total_v; i++) {
		//	print_vex3d(&dest->v[i]);
		}
		
		Polygon2D out;
		
	}
	
	//===============================
	for(i = 0; i < out_pos - 1; i++) {
		if(out[i] == out[i + 1] - 1) {
			//printf("Case\n");
			dest->draw[out[i]] = 0;
		}
		else {
			//error("ERRORX");
			
			if(out[i] != 0 || out[i + 1] != dest->total_v - 1) { 
			
				printf("A: %d, B: %d\n", out[i], out[i + 1]);
				
				dest->draw[out[i]] = 0;
				dest->draw[out[i + 1]] = 0;
			}
				
			
		}
	}
	
	if(out_pos > 0 && out[0] == 0 && out[out_pos - 1] == dest->total_v - 1) {
		dest->draw[dest->total_v - 1] = 0;
		//printf("CASE\n");
	}
	
	
	
	
	if(out_pos != 0 && out_pos != 2) {
		return 0;
		
		
		PortRestore();
		clrscr();
		
		print_polygon(poly);
		printf("Total: %d\n", out_pos);
		
		while(1);
	}
	
	
	
	errorif(out_pos != 0 && out_pos != 2, "Wrong out pos: %d\n", out_pos);
	
	return dest->total_v > 2;	
}

// Clips a polygon against the entire view frustum
// This routine requires two temporary polygons, one of which the
// final polygon will be in. This returns the address of which one it
// is
char clip_polygon_to_frustum(Polygon* src, Frustum* f, Polygon* dest) {
#if 1
	Polygon temp[2];
	int current_temp = 0;
	Polygon* poly = src;
	int i;
	
	ADDR(src);
	ADDR(f);
	ADDR(dest);
	
	
#if 1
	for(i = 0; i < f->total_p - 1; i++) {
		//errorif(poly->total_v < 3, "Invalid clip poly");
		//return clip_polygon_to_plane(src, &f->p[FRUSTUM_TOP], dest);
		
		//if(i == PLANE_LEFT || i == PLANE_RIGHT)
		//	continue;
			
		if(!clip_polygon_to_plane(poly, &f->p[i], &temp[current_temp])) {
			return 0;
		}
		
		poly = &temp[current_temp];
		current_temp = !current_temp;
	}
#endif
	
	//return poly->total_v > 2;
	return clip_polygon_to_plane(poly, &f->p[f->total_p - 1], dest);
	
#endif
}


//=========================================================================

// Returns the sign of a value
short signof(short x) {
	if(x == 0) return 0;
	return (x < 0 ? -1 : 1);
}

// Evalulates the line y=mx + b at the given x
// Returns the value as a short
inline short eval_line(Line2D* line, short x) {
	return (((long)line->slope * x) >> FRAC_BITS) + line->b;
}

// Evalulates the line y=mx + b at the given x
// Returns the value as a long
inline long eval_line_long(Line2D* line, long x) {
	return ((((long)line->slope * x) >> FRAC_BITS) >> EVAL_BITS) + line->b;
}

// Determines whether point is on the "correct" side of a line
// i.e. given a region bounded by lines, this checks if for one of
// the lines the point is on the inside
char point_valid_side(Line2D* line, Vex2D* point) {	
	char sign;
	
	ADDR(line);
	ADDR(point);
	
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

// Calculates information about a line, including its slope, y-intercept,
// and which side of the line is considered the inside
inline void get_line_info(Line2D* dest, Vex2D* start, Vex2D* end, Vex2D* center) {
	short dx = end->x - start->x;
	short dy = end->y - start->y;
	
	
	ADDR(dest);
	ADDR(start);
	ADDR(end);
	ADDR(center);
	
	if(abs(dx) < 2) {
vertical:
		dest->slope = VERTICAL_LINE;
		dest->b = start->x;
		dest->sign = signof(center->x - start->x);
		//dest->start = min(start->y, end->y);
		//dest->end = max(start->y, end->y);
	}
	else {
		dest->slope = ((long)dy << FRAC_BITS) / dx;
		
		if(abs(dest->slope) > 0x6FFF)
			goto vertical;
		
		//if(abs(dest->slope) > 5000)//5120)
		//	goto vertical;
		
		dest->b = start->y - (((long)dest->slope * start->x) >> FRAC_BITS);
		dest->sign = signof(center->y - eval_line(dest, center->x));
		//dest->start = min(start->x, end->x);
		//dest->end = max(start->x, end->x);
	}
}

// Determines if two lines intersects, and if they do, calculates the
// point of intersection
// Returns whether they intersect
char line2d_intersect(Line2D* a, Line2D* b, Vex2D* res, Vex2D* center) {
	//Line2D a;
	//Line2D b;
	
	ADDR(a);
	ADDR(b);
	ADDR(res);
	ADDR(center);
	
	//get_line_info(a, x0, x1, center);
	//get_line_info(b, y0, y1, center);
	
	if(abs(a->slope - b->slope) < 10)
		return 0;
	else if(b->slope == VERTICAL_LINE) {
		SWAP(a, b);
	}
	
	if(a->slope == VERTICAL_LINE) {
		res->x = a->b;
		res->y = eval_line(b, a->b);
		
		return 1;
		//return res->y >= a->start && res->y <= a->end;
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
		//return res->x >= a->start && res->x <= a->end;
	}
	
	
	
}

// Adds a point to a polygon if adding so wouldn't cause duplicate
// points
// Returns 2 if the points was degenerate and 1 other wise
char add_point(Polygon2D* p, Vex2D* point, Line2D* line, char draw) {
	char clipped = 1;
	
	ADDR(p);
	ADDR(point);
	ADDR(line);
	
	if(p->total_v != 0) {
		if((point->x == p->p[0].v.x && point->y == p->p[0].v.y) ||
			(point->x == p->p[p->total_v - 1].v.x && point->y == p->p[p->total_v - 1].v.y))
				clipped = 2;
	}
	
	
	if(clipped != 2) {
		p->p[p->total_v] = (Point){1, *point};
		p->line[p->total_v] = *line;
		p->line[p->total_v++].draw = draw;
	}
	
	return clipped;
}

// Clips a polygon against a single edge
// TODO: remove center as a parameter
void polygon_clip_edge(Polygon2D* p, Line2D* edge, Polygon2D* dest, Vex2D* center, char allow_extra_clip) {
	int point;
	int next_point;
	char next;
	Vex2D clip_pos;
	short out[20];
	short out_pos = 0;
	int i;
	
	ADDR(p);
	ADDR(edge);
	ADDR(dest);
	ADDR(center);
	
	char side = point_valid_side(edge, &p->p[0].v);
	char next_side;
	char clipped;
	char current_clipped;
	
	short last_added = -1;
	
	char skip_next = 0;
	short skip_point;
	
	short last_added_point = -1;
	
	dest->total_v = 0;
	
	for(point = 0; point < p->total_v; point++) {
		Line2D* line = &p->line[point];
		
		
		if(dest->total_v == MAX_POINTS - 1)
			return;
		
		//errorif(dest->total_v >= MAX_POINTS - 1, "Too many points");
		
		next_point = (point + 1) % p->total_v;
		next_side = point_valid_side(edge, &p->p[next_point].v);
		
		current_clipped = 0;
		
		// Check for degenerate points
		if(p->p[point].v.x == p->p[next_point].v.x && p->p[point].v.y == p->p[next_point].v.y) {
			//side = next
			continue;
		}
		
		if(side != -1 && (!skip_next || skip_next == 3)) {
			//dest->p[dest->total_points] = p->p[point];
			//dest->line[dest->total_points++] = *line;
			
			current_clipped = add_point(dest, &p->p[point].v, line, p->line[point].draw);
			
			if(current_clipped != 2) {
				last_added_point = dest->total_v - 1;
			
				//if(skip_next == 3) {
				//	skip_point = last_added_point;
				//	printf("Skip point %d: \n", skip_point);
				//}
			}
		}
		
		// If we're not drawing the line connecting the current point and the next, and we're
		// not drawing the line connecting the next point and the next next point, we can
		// remove it
		if(!p->line[point].draw && !p->line[next_point].draw && p->total_v >= 4 && allow_extra_clip) {
			// Skip over the next point
			
			if(last_added_point == -1) {
				printf("uninitialized\n");
			}
			
			if(!skip_next && last_added_point != -1) {
				skip_next = 2;
				skip_point = last_added_point;
				
				//if(last_added_point == 1000) {
				//	skip_next = 3;
				//	printf("unitialized\n");
				//}
			}
			
			//errorif(last_added_point == 1000, "Unitialized last point!");
		}
		
		
		
		clipped = 0;
		if(side + next_side == 0 && side && (!skip_next || skip_next == 3)) {
			if(line2d_intersect(line, edge, &clip_pos, center)) {
				// Check if the current point is degenerate...
				//if(dest->total_points != 0) {
				//if(clip_pos.x != 
				
				
				//dest->p[dest->total_points] = (Point){1, clip_pos};
				//dest->line[dest->total_points++] = p->line[point];
					
				//out[out_pos++] = dest->total_points++;
				clipped = add_point(dest, &clip_pos, &p->line[point], p->line[point].draw);
				
				if(clipped != 2)
					last_added_point = dest->total_v - 1;
				
				//if(skip_next == 3) {
				//	skip_point = last_added_point;
				//	printf("Skip point %d: \n", skip_point);
				//}
			}
		}
		
		
		if(clipped != 2 && current_clipped != 2) {
			if(clipped || side == 0) {
				out[out_pos++] = dest->total_v - 1;
			}
			
			//if(side != next_side && min(side, next_side) <= 0) {
			//	out[out_pos++] = dest->total_v - 1;
			//}
			
			
			/*if(side + next_side == 0 && side) {
				printf("point: %d next_point: %d\n", point, next_point);
				printf("side: %d next_side: %d\n", side, next_side);
				
				if(side == 1)
					out[out_pos++] = dest->total_v - 1;
				else
					out[out_pos++] = dest->total_v - 1;
			}*/
			
		}
		else {
			//printf("Degenerate\n");
		}
		
		if(skip_next == 2) {
			skip_next = 1;
			continue;
		}
		
		if(skip_next) {
			get_line_info(&dest->line[last_added_point], &dest->p[last_added_point].v, &dest->p[next_point].v, &p->center);
			
			//errorif(dest->line[last_added_point].draw, "Should not draw...");
			
			skip_next = 0;
		}
		
		side = next_side;
	}
	
	//for(i = 0; i < out_pos; i++)
		//printf("Out: %d\n", out[i]);
	
	//errorif((out_pos % 2), "Invalid out pos: %d\n", out_pos);
	
	char a = 0;
	
	for(i = 0; i < out_pos - 1; i++) {
		if(out[i] == out[i + 1] - 1) {
			//printf("Case: %d, %d\n", out[i], out[i + 1]);
			dest->line[out[i]] = *edge;
			a = 1;
			dest->line[out[i]].draw = 0;
		}
	}
	
	if(out_pos > 0 && out[0] == 0 && out[out_pos - 1] == dest->total_v - 1) {
		dest->line[dest->total_v - 1] = *edge;
		dest->line[dest->total_v - 1].draw = 0;
		//printf("CLIP LAST\n");
		
		//errorif(a, "BOTH");
	}
	
	dest->center = p->center;
	
	//printf("out_pos: %d\n", out_pos);
	
	//print_polygon2d(dest);
	
	
	
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
		
		
		
		
	//	error("Invalid out B\n");
}

char clip_polygon(Polygon2D* p, Polygon2D* clip, Polygon2D* dest, char allow_extra_clip) {
	int i;
	Vex2D center = {0, 0};
	
	Polygon2D temp[2];
	Polygon2D* poly = p;
	short current_temp = 0;
	
	for(i = 0; i < clip->total_v - 1; i++) {
		//errorif(poly->total_v < 3, "Invalid clip poly");
		//return clip_polygon_to_plane(src, &f->p[FRUSTUM_TOP], dest);
		polygon_clip_edge(poly, &clip->line[i], &temp[current_temp], &center, allow_extra_clip);
		
		if(temp[current_temp].total_v == 0) {
			dest->total_v = 0;
			return 0;
		}
		
		poly = &temp[current_temp];
		current_temp = !current_temp;
	}
	
	polygon_clip_edge(poly, &clip->line[clip->total_v - 1], dest, &center, allow_extra_clip);
	
	return dest->total_v != 0;
}

#if 0
// Clips a polygon against another polygon
// Note: both the clipping polygon and polygon to be clipped MUST be convex!
Polygon2D* clip_polygon(Polygon2D* p, Polygon2D* clip, Polygon2D* temp_a, Polygon2D* temp_b, char allow_extra_clip) {
	Vex2D center = {0, 0};
	int i;
	Polygon2D *p1 = temp_a;
	Polygon2D *p2 = temp_b;
	
	ADDR(temp_a);
	ADDR(temp_b);
	ADDR(p);
	ADDR(clip);
	
	// Clip against the first edge
	polygon_clip_edge(p, &clip->line[clip->total_v - 1], p2, &center, allow_extra_clip);
	
	
	//for(i = 0; i < p2->total_v; i++)
	//	p2->line[i].draw = 0;
	
	
	//p2->line[0].draw = 1;
	//p2->line[1].draw = 1;
	
	//printf("Total v: %d\n", p2->total_v);
	
	int d = 0;
	
	for(i = 0; i < clip->total_v - 1; i++) {
		//print_polygon(p2);
		SWAP(p1, p2);
		
		if(p1->total_v == 0) {
			p2->total_v = 0;
			break;
		}
		
		polygon_clip_edge(p1, &clip->line[i], p2, &center, allow_extra_clip);
	}
	
	return p2;
}
#endif

// A simple interative function for debugging the polygon clipper
void test_polygon_clipper(RenderContext* context) {
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
	
	
	
	Polygon2D temp_a, temp_b;
	
	Polygon2D pp;
	make_polygon2d(p, 4, &pp);
	
	Polygon2D clipp;
	make_polygon2d(clip, 4, &clipp);
	
	short p_pos = 0;
	
	
	int cx = 39, cy = 63;
	unsigned short key;
	
	draw_polygon(&clipp, context);
	
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
	
	make_polygon2d(p, p_pos, &pp);
	
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
	
	
	draw_polygon(&pp, context);
	ngetchx();
	draw_polygon(&clipp, context);
	ngetchx();
	
	Polygon2D* res = NULL;// = clip_polygon(&pp, &clipp, &temp_a, &temp_b, 0);
	
	clrscr();
	draw_polygon(&clipp, context);
	draw_polygon(res, context);
	ngetchx();
	
	do {
		clrscr();
		draw_polygon(res, context);
		//draw_polygon(&clipp);
		ngetchx();
		
		clrscr();
		draw_polygon(&pp, context);
		draw_polygon(&clipp, context);
	} while(ngetchx() != KEY_ESC);
}
