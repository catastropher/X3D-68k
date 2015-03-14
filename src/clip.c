// C Source File
// Created 3/5/2015; 8:39:09 AM

#include "geo.h"
#include "math.h"
#include "console.h"
#include "error.h"

#include <tigcclib.h>

// Clips a polygon against a plane. Returns whether a valid polygon remains.
// TODO: keep track of which edges have been clipped so we know which
// part of the polygon still needs to be drawn
char clip_polygon_to_plane(Polygon* poly, Plane* plane, Polygon* dest) {
	short i;
	short next_point;
	short in, next_in;
	short dot, next_dot;
	long t;
	
	dot = dot_product(&poly->v[0], &plane->normal);
	in = (dot >= plane->d);
	
	
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
			errorif(next_dot - dot == 0, "Clip div by 0");
			
			t = FIXDIV8(plane->d - dot, next_dot - dot);
			
			errorif(abs(plane->d - dot) > 32767, "plane->d too big");
			errorif(abs(next_dot - dot) > 32767, "next_dot too big");
			
			errorif(abs(t) > 32767, "Invalid clip t");
			//errorif(t == 0, "t == 0");
			
			//printf("Dist: %d\n", dot + plane->d);
			//printf("T: %ld Z: %d\n", t, poly->v[i].z);
			
			dest->v[dest->total_v].x = poly->v[i].x + FIXMUL8(((long)poly->v[next_point].x - poly->v[i].x), t);
			dest->v[dest->total_v].y = poly->v[i].y + FIXMUL8(((long)poly->v[next_point].y - poly->v[i].y), t);
			dest->v[dest->total_v].z = poly->v[i].z + FIXMUL8(((long)poly->v[next_point].z - poly->v[i].z), t);
			
			dest->draw[dest->total_v] = poly->draw[i];
			
			//printf("Dest z: %d\n", dest->v[dest->total_v].z);
			//printf("Should be: %d\n", -plane->d);
			
			//errorif(dest->v[dest->total_v].z < DIST_TO_NEAR_PLANE / 2, "Invalid clip: %d", dest->v[dest->total_v].z);
			
			dest->total_v++;
		}
		
		if(dest->total_v > 1) {
			if(!in && next_in) {
				dest->draw[dest->total_v - 1] = 0;
			}
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
	
	
#if 1
	for(i = 0; i < f->total_p - 1; i++) {
		//errorif(poly->total_v < 3, "Invalid clip poly");
		//return clip_polygon_to_plane(src, &f->p[FRUSTUM_TOP], dest);
		
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
char signof(short val) {
	return (val < 0 ? -1 : val > 0);
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
		
		if(abs(dest->slope) > VERTICAL_LINE)
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
void polygon_clip_edge(Polygon2D* p, Line2D* edge, Polygon2D* dest, Vex2D* center) {
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
	char current_clipped;
	
	short last_added = -1;
	
	dest->total_v = 0;
	
	for(point = 0; point < p->total_v; point++) {
		Line2D* line = &p->line[point];
		
		next_point = (point + 1) % p->total_v;
		next_side = point_valid_side(edge, &p->p[next_point].v);
		
		current_clipped = 0;
		
		if(side != -1) {
			//dest->p[dest->total_points] = p->p[point];
			//dest->line[dest->total_points++] = *line;
			
			current_clipped = add_point(dest, &p->p[point].v, line, p->line[point].draw);
		}
		
		clipped = 0;
		if(side + next_side == 0 && side) {
			if(line2d_intersect(line, edge, &clip_pos, center)) {
				// Check if the current point is degenerate...
				//if(dest->total_points != 0) {
				//if(clip_pos.x != 
				
				
				//dest->p[dest->total_points] = (Point){1, clip_pos};
				//dest->line[dest->total_points++] = p->line[point];
					
				//out[out_pos++] = dest->total_points++;
				clipped = add_point(dest, &clip_pos, &p->line[point], p->line[next_point].draw);
			}
		}
		
		if(clipped != 2 && current_clipped != 2) {
			if(clipped || side == 0) {
				out[out_pos++] = dest->total_v - 1;
			}
		}
		
		// If we clipped this point, we should not draw the line to the next point
		
	#if 1
		if(dest->total_v > 1) {
			if(side == -1 && next_side == 1) {
				dest->line[dest->total_v - 1].draw = 0;
			}
		}
	#endif
		
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
	
	if(out_pos > 0 && out[0] == 0 && out[out_pos - 1] == dest->total_v - 1) {
		dest->line[dest->total_v - 1] = *edge;
		dest->line[dest->total_v - 1].draw = 0;
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
		
		
		
		
	//	error("Invalid out B\n");
}

// Clips a polygon against another polygon
// Note: both the clipping polygon and polygon to be clipped MUST be convex!
Polygon2D* clip_polygon(Polygon2D* p, Polygon2D* clip, Polygon2D* temp_a, Polygon2D* temp_b) {
	Vex2D center = {0, 0};
	int i;
	Polygon2D *p1 = temp_a;
	Polygon2D *p2 = temp_b;
	
	// Clip against the first edge
	polygon_clip_edge(p, &clip->line[clip->total_v - 1], p2, &center);
	
	
	for(i = 0; i < clip->total_v - 1; i++) {
		//print_polygon(p2);
		SWAP(p1, p2);
		
		if(p1->total_v == 0) {
			p2->total_v = 0;
			break;
		}
		
		polygon_clip_edge(p1, &clip->line[i], p2, &center);
	}
	
	return p2;
}


