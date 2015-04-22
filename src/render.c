// C Source File
// Created 3/7/2015; 6:28:25 PM

#include <tigcclib.h>

#include "geo.h"
#include "extgraph.h"
#include "error.h"

// The table of the vertex numbers for each face
// The last vertex of each face is duplicated to make loops easy
const int cube_vertex_tab[6][5] = {
	{2, 1, 0, 3, 2},
	{6, 7, 4, 5, 6},
	{0, 1, 5, 4, 0},
	{2, 3, 7, 6, 2},
	{2, 6, 5, 1, 2},
	{3, 0, 4, 7, 3}
};

// Given a vertex id a and id b, edge_table[a][b] gets the edge id between a and b
char edge_table[8][8];
short edge_vertex_table[12][2];

// Given the face id, this gets the bitset of the edges in that face
unsigned short edge_face_table[6];

// Gets the child connected to the given face of a cube
inline short cube_get_child(Cube* c, short face) {
	return c->cube[face] >> 3;
}



/**
 * Initializes a render context.
 *
 * @param w		- the width of the rendering context
 * @param h		- the height of the rendering context
 * @param x		- the x position in the video buffer of the context
 * @param y		- the y position in the video buffer of the context
 * @param fov	- the angle of the field of view
 * @param c		- pointer to the destination render context
 *
 * @return nothing
 */
void init_render_context(short w, short h, short x, short y, unsigned char fov, X3D_RenderContext* c) {
	ADDR(c);

	c->w = w;
	c->h = h;
	c->x = x;
	c->y = y;
	c->fov = fov;
	c->center_x = w / 2;
	c->center_y = h / 2;

	// Calulate the distance to the project plane (which is the scale)
	// dist = (w / 2) / tan(fov / 2)
	c->dist = FIXDIV8(w / 2, tanfp(fov / 2));
	c->frame = 0;

	// Calculate the unrotated planes of the view frustum
	calculate_frustum_plane_normals(c);
}

long line_count;

/**
 * Clamps a point so that it's on screen. Note that this is only best when the
 * point is close to the screen boundary.
 *
 * @param v - the 2D point to clamp
 *
 * @return nothing
 *
 * @todo This should clamp to the rendering context, not the screen!
 */
void clamp_point(Vex2D* v) {
	ADDR(v);

	if(v->x < 0)
		v->x = 0;
	else if(v->x >= (short)LCD_WIDTH)
		v->x = LCD_WIDTH - 1;

	if(v->y < 0)
		v->y = 0;
	else if(v->y >= (short)LCD_HEIGHT)
		v->y = LCD_HEIGHT - 1;
}

// Draws a polygon
void draw_polygon(Polygon2D* p, X3D_RenderContext* context) {
	int i, next;

	ADDR(p);
	ADDR(context);

	clamp_point(&p->p[0].v);

	for(i = 0; i < p->total_v; i++) {
		next = i + 1;

		if(next == p->total_v)
			next = 0;

		clamp_point(&p->p[next].v);

		if(p->line[i].draw) {
			FastLine_Draw_R(context->screen, p->p[i].v.x, p->p[i].v.y, p->p[next].v.x, p->p[next].v.y);
			line_count++;
		}
	}
}

short cube_id;

short recursion_depth;
short max_recursion_depth;

// Draws a transparent 3D polygon and clips it against the clipping region
void draw_clipped_polygon3D(Polygon3D* poly, X3D_RenderContext* context, Polygon2D* clip) {
	Polygon3D clipped, rotated;
	Polygon2D projected, final;
	clip_polygon_to_frustum(poly, &context->frustum, &clipped);

	int i;
	Vex3D ncam_pos = {-context->cam.pos.x, -context->cam.pos.y, -context->cam.pos.z};

	for(i = 0; i < clipped.total_v; i++) {
		Vex3D temp;
		add_vex3d(&clipped.v[i], &ncam_pos, &temp);
		rotate_vex3d(&temp, &context->cam.mat, &rotated.v[i]);

		rotated.draw[i] = clipped.draw[i];
	}

	rotated.total_v = clipped.total_v;

	project_polygon3d(&rotated, context, &projected);

	for(i = 0; i < projected.total_v; i++)
		projected.line[i].draw = rotated.draw[i];

	clip_polygon(&projected, clip, &final, 0);

	if(final.total_v > 1)
		draw_polygon(&final, context);
}

// Draws a 3D line
// Currently this uses a terrible method; it makes a triangle with one edge visible
// and throws it at draw_clipped_polygon3D(!)
void draw_3D_line(Vex3D* a, Vex3D* b, X3D_RenderContext* context, Polygon2D* clip) {
	Polygon3D p;

	p.total_v = 3;
	p.v[0] = *a;
	p.v[1] = *b;
	p.v[2] = *a;

	p.v[2].x += 100;
	p.v[2].y  += 100;

	p.draw[0] = 1;
	p.draw[1] = 0;
	p.draw[2] = 0;

	draw_clipped_polygon3D(&p, context, clip);
}

// Globals variables related to the light bridge
extern Vex3D switch_pos;
extern char switch_active;
extern Vex3D bridge_left_start;
extern Vex3D bridge_left_end;
extern Vex3D bridge_right_start;
extern Vex3D bridge_right_end;
extern short bridge_t;
extern char extend_bridge;
extern short bridge_shimmer_t;


void render_cube(Cube* c, X3D_RenderContext* context, Polygon2D* clip, short id) {
	Vex3D rot[8];
	int i, d;
	Vex3D ncam_pos = {-context->cam.pos.x, -context->cam.pos.y, -context->cam.pos.z};

	//++line_count;

	c->edge_bits |= (1 << 15);

	if(recursion_depth > max_recursion_depth)
		max_recursion_depth = recursion_depth;

	if(recursion_depth > 20) {
		return;
	}

	int visit = 0;

	// Used for debugging purposes to determine the current cube being rendered
	cube_id = id;

	// Originally I didn't put these in a union, but I was getting stack overflows so
	// I really had no choice
	union {
		Polygon3D poly3D;
		Polygon2D out_2d;
	} set_a;

	union {
		Polygon3D poly_out;
		Polygon2D dest;
	} set_b;

	short faces_drawn = 0;

	for(i = 0; i < 6; i++) {
		cube_get_face(c->v, i, set_a.poly3D.v);

		set_a.poly3D.total_v = 4;

		// Which edges of the current face we need to draw
		unsigned char draw_edges = 0;


		// If the angle between the view directions and the polygon normal < 45 deg,
		// we're going to assume that the polygon isn't visible
	#if 1
		if(dot_product(&c->normal[i], &context->cam.dir) > 23170)
			continue;
	#endif

		// If we're on the wrong side of the plane, it must be invisible (backface culling)
		short dist = dist_to_plane(&c->normal[i], &context->cam.pos, &c->v[cube_vertex_tab[i][0]]);

	#if 1
		if(dist > 10)
			continue;
	#endif

		// Determine which of the edges need to be drawn
		for(d = 0; d < 4; d++) {
			short a = cube_vertex_tab[i][d];
			short b = cube_vertex_tab[i][d + 1];

			draw_edges = (draw_edges >> 1) | ((unsigned short)((c->edge_bits & (1 << edge_table[a][b])) == 0) << 3);

			//draw_edges = (draw_edges >> 1) | ((!((c->invisible_edges >> edge_table[a][b]) & 1)) << 3);

			c->edge_bits |= (1 << edge_table[a][b]);
		}

	#if 1
		// If none of the edges need to be drawn and this isn't a portal, we can skip clipping it
		if(draw_edges == 0 && c->cube[i] == -1)
			continue;
	#endif

		// Now that we know which edges need to be drawn, copy it over to the 3D polygon
		for(d = 0; d< set_a.poly3D.total_v; d++) {
			set_a.poly3D.draw[d] = draw_edges & 1;
			draw_edges >>= 1;
		}

		// Clip the face against the viewing frustum
		char draw_face = clip_polygon_to_frustum(&set_a.poly3D, &context->frustum, &set_b.poly_out);

		Polygon2D* res = NULL;

		if(draw_face) {

			// Is this code being duplicated??? Hmm....
			// Rotate the points of the face
			for(d = 0; d < set_b.poly_out.total_v; d++) {
				Vex3D temp;
				add_vex3d(&set_b.poly_out.v[d], &ncam_pos, &temp);

				rotate_vex3d(&temp, &context->cam.mat, &set_b.poly_out.v[d]);
			}

			// Project the points onto the screen
			project_polygon3d(&set_b.poly_out, context, &set_a.out_2d);

			for(d = 0; d < set_b.poly_out.total_v; d++)
				set_a.out_2d.line[d].draw = set_b.poly_out.draw[d];

			res = &set_b.dest;

			// Clip the edges to the bounds of the clipping region
			clip_polygon(&set_a.out_2d, clip, &set_b.dest, 0);//c->cube[i] == -1);

			// Draw the polygon
			//if(id == 5 || id == 10)
				draw_polygon(res, context);

			if(id == 10) {
				//printf("Drew: %d\n", i);
			}

			++faces_drawn;
		}

		// If this face has a child cube, we may need to render it
		if(c->cube[i] != -1) {
			short cube_face = c->cube[i] & 0b111;
			short child_id = cube_get_child(c, i);
			Cube* next_cube = &cube_tab[child_id];

			Polygon2D* new_clip;

			if(draw_face) {
				new_clip = res;
			}
			else {
				new_clip = clip;
			}

			if(id == 5)
				++visit;


			// If we're really close to the portal, go ahead and just use the original clipping
			// region (the new clipping region may have singularaties)
		#if 1
			if(dist > -120 && dist <= 10 && id == context->cam.current_cube) {
				draw_face = 1;
				new_clip = clip;

			}
		#endif

			if(draw_face && new_clip->total_v > 2 && recursion_depth < 20) {
				// Pass over which edges have already been drawn

				//if(c->cube[i] == 10) {
				//	printf("Made it to here\n");
				//}

				cube_pass_edges(context, next_cube, cube_face);

				char should_draw = !(next_cube->edge_bits & (1 << 15));


				if(should_draw) {

					int j;

					for(j = 0; j < new_clip->total_v; j++) {
						new_clip->line[j].draw = 1;
					}

					//draw_polygon(new_clip, context);

					c->render_bits |= (1 << i);
					++recursion_depth;
					render_cube(next_cube, context, new_clip, child_id);
					--recursion_depth;
				}
			}
		}
	}

#if 1
	if(id == 36) {
		// Draw the light brigde switch
		Vex3D center = switch_pos;

		printf("Center: ");
		print_vex3d(&center);

		Polygon3D poly;

		poly.total_v = 4;

		int WIDTH = 40;
		int HEIGHT = 40;

		poly.v[0] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z - WIDTH / 2};
		poly.v[1] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[2] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[3] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z - WIDTH / 2};

		for(i = 0; i < 4; i++) {
			poly.draw[i] = 1;
		}

		draw_clipped_polygon3D(&poly, context, clip);

		WIDTH = 20;
		HEIGHT = 20;

		poly.v[0] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z - WIDTH / 2};
		poly.v[1] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[2] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[3] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z - WIDTH / 2};

		for(i = 0; i < 4; i++) {
			poly.draw[i] = 1;
		}

		draw_clipped_polygon3D(&poly, context, clip);

		if(switch_active) {
			WIDTH = 15;
			HEIGHT = 15;

			poly.v[0] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z - WIDTH / 2};
			poly.v[1] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z + WIDTH / 2};
			poly.v[2] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z - WIDTH / 2};
			poly.v[3] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z + WIDTH / 2};

			poly.draw[0] = 1;
			poly.draw[1] = 0;
			poly.draw[2] = 1;
			poly.draw[3] = 0;

			draw_clipped_polygon3D(&poly, context, clip);

		}
	}
	else if(id == 37) {
		if(extend_bridge) {
			Vex3D end_left, end_right;

			param_vex3d(&bridge_left_end, &bridge_left_start, bridge_t, &end_left);
			param_vex3d(&bridge_right_start, &bridge_right_end, bridge_t, &end_right);

			draw_3D_line(&bridge_left_end, &end_left, context, clip);
			draw_3D_line(&bridge_right_start, &end_right, context, clip);
		}

		if(bridge_shimmer_t >= 0) {
			Vex3D left, right;

			param_vex3d(&bridge_left_start, &bridge_left_end, bridge_shimmer_t, &left);
			param_vex3d(&bridge_right_start, &bridge_right_end, bridge_shimmer_t, &right);

			draw_3D_line(&left, &right, context, clip);
		}
	}
#endif
}

//======================================================================================
void render_cube_clip3D(Cube* c, X3D_RenderContext* context, Frustum* clip, short id) {
	Vex3D rot[8];
	int i, d;
	Vex3D ncam_pos = {-context->cam.pos.x, -context->cam.pos.y, -context->cam.pos.z};

	Frustum new_frustum;

	//++line_count;

	c->edge_bits |= (1 << 15);

	if(recursion_depth > max_recursion_depth)
		max_recursion_depth = recursion_depth;

	if(recursion_depth > 7) {
		return;
	}

	int visit = 0;

	// Used for debugging purposes to determine the current cube being rendered
	cube_id = id;

	// Originally I didn't put these in a union, but I was getting stack overflows so
	// I really had no choice
	union {
		Polygon3D poly3D;
		Polygon2D out_2d;
	} set_a;

	union {
		Polygon3D poly_out;
		Polygon2D dest;
	} set_b;

	short faces_drawn = 0;

	for(i = 0; i < 6; i++) {
		cube_get_face(c->v, i, set_a.poly3D.v);

		set_a.poly3D.total_v = 4;

		// Which edges of the current face we need to draw
		unsigned char draw_edges = 0;


		// If the angle between the view directions and the polygon normal < 45 deg,
		// we're going to assume that the polygon isn't visible
	#if 1
		if(dot_product(&c->normal[i], &context->cam.dir) > 23170)
			continue;
	#endif

		// If we're on the wrong side of the plane, it must be invisible (backface culling)
		short dist = dist_to_plane(&c->normal[i], &context->cam.pos, &c->v[cube_vertex_tab[i][0]]);

	#if 1
		if(dist > 10)
			continue;
	#endif

		// Determine which of the edges need to be drawn
		for(d = 0; d < 4; d++) {
			short a = cube_vertex_tab[i][d];
			short b = cube_vertex_tab[i][d + 1];

			draw_edges = (draw_edges >> 1) | ((unsigned short)((c->edge_bits & (1 << edge_table[a][b])) == 0) << 3);

			//draw_edges = (draw_edges >> 1) | ((!((c->invisible_edges >> edge_table[a][b]) & 1)) << 3);

			c->edge_bits |= (1 << edge_table[a][b]);
		}

	#if 1
		// If none of the edges need to be drawn and this isn't a portal, we can skip clipping it
		if(draw_edges == 0 && c->cube[i] == -1)
			continue;
	#endif

		// Now that we know which edges need to be drawn, copy it over to the 3D polygon
		for(d = 0; d< set_a.poly3D.total_v; d++) {
			set_a.poly3D.draw[d] = draw_edges & 1;
			draw_edges >>= 1;
		}

		// Clip the face against the viewing frustum
		char draw_face = clip_polygon_to_frustum(&set_a.poly3D, clip, &set_b.poly_out);

		Polygon2D* res = NULL;

		Polygon3D copy = set_b.poly_out;

		if(draw_face) {

			// Is this code being duplicated??? Hmm....
			// Rotate the points of the face
			for(d = 0; d < set_b.poly_out.total_v; d++) {
				Vex3D temp;
				add_vex3d(&set_b.poly_out.v[d], &ncam_pos, &temp);

				rotate_vex3d(&temp, &context->cam.mat, &set_b.poly_out.v[d]);
			}

			// Project the points onto the screen
			project_polygon3d(&set_b.poly_out, context, &set_a.out_2d);

			for(d = 0; d < set_b.poly_out.total_v; d++)
				set_a.out_2d.line[d].draw = set_b.poly_out.draw[d];

			res = &set_b.dest;

			// Clip the edges to the bounds of the clipping region
			// EDIT: no longer necessary
			// clip_polygon(&set_a.out_2d, clip, &set_b.dest, 0);

			// Draw the polygon
			draw_polygon(&set_a.out_2d, context);

			//LCD_restore(context->screen);

			if(id == 10) {
				//printf("Drew: %d\n", i);
			}

			++faces_drawn;
		}

		// If this face has a child cube, we may need to render it
		if(c->cube[i] != -1) {
			short cube_face = c->cube[i] & 0b111;
			short child_id = cube_get_child(c, i);
			Cube* next_cube = &cube_tab[child_id];

			Polygon2D* new_clip;
			Frustum* frustum;

		#if 1
			if(draw_face) {
				frustum = &new_frustum;
			}
			else {
				frustum = clip;
			}

			if(id == 5)
				++visit;
		#endif


			// If we're really close to the portal, go ahead and just use the original clipping
			// region (the new clipping region may have singularaties)
		#if 1
			if(dist > -120 && dist <= 10 && id == context->cam.current_cube) {
				draw_face = 1;
				frustum = clip;
			}
		#endif

			if(draw_face && /*new_clip->total_v > 2 && */recursion_depth < 20) {
				// Pass over which edges have already been drawn

				//if(c->cube[i] == 10) {
				//	printf("Made it to here\n");
				//}

				cube_pass_edges(context, next_cube, cube_face);

				char should_draw = !(next_cube->edge_bits & (1 << 15));

				if(should_draw) {

					int j;

					//for(j = 0; j < new_clip->total_v; j++) {
						//new_clip->line[j].draw = 1;
					//}

					//draw_polygon(new_clip, context);

					c->render_bits |= (1 << i);
					++recursion_depth;

					if(frustum == clip || copy.total_v > 2) {
						if(frustum != clip)
							construct_frustum_from_polygon3D(&copy, context, &new_frustum);

						render_cube_clip3D(next_cube, context, frustum, child_id);
					}

					--recursion_depth;
				}
			}
		}
	}

#if 0
	if(id == 36) {
		// Draw the light brigde switch
		Vex3D center = switch_pos;

		printf("Center: ");
		print_vex3d(&center);

		Polygon3D poly;

		poly.total_v = 4;

		int WIDTH = 40;
		int HEIGHT = 40;

		poly.v[0] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z - WIDTH / 2};
		poly.v[1] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[2] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[3] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z - WIDTH / 2};

		for(i = 0; i < 4; i++) {
			poly.draw[i] = 1;
		}

		draw_clipped_polygon3D(&poly, context, clip);

		WIDTH = 20;
		HEIGHT = 20;

		poly.v[0] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z - WIDTH / 2};
		poly.v[1] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[2] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z + WIDTH / 2};
		poly.v[3] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z - WIDTH / 2};

		for(i = 0; i < 4; i++) {
			poly.draw[i] = 1;
		}

		draw_clipped_polygon3D(&poly, context, clip);

		if(switch_active) {
			WIDTH = 15;
			HEIGHT = 15;

			poly.v[0] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z - WIDTH / 2};
			poly.v[1] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z + WIDTH / 2};
			poly.v[2] = (Vex3D){center.x, center.y + HEIGHT / 2, center.z - WIDTH / 2};
			poly.v[3] = (Vex3D){center.x, center.y - HEIGHT / 2, center.z + WIDTH / 2};

			poly.draw[0] = 1;
			poly.draw[1] = 0;
			poly.draw[2] = 1;
			poly.draw[3] = 0;

			draw_clipped_polygon3D(&poly, context, clip);

		}
	}
	else if(id == 37) {
		if(extend_bridge) {
			Vex3D end_left, end_right;

			param_vex3d(&bridge_left_end, &bridge_left_start, bridge_t, &end_left);
			param_vex3d(&bridge_right_start, &bridge_right_end, bridge_t, &end_right);

			draw_3D_line(&bridge_left_end, &end_left, context, clip);
			draw_3D_line(&bridge_right_start, &end_right, context, clip);
		}

		if(bridge_shimmer_t >= 0) {
			Vex3D left, right;

			param_vex3d(&bridge_left_start, &bridge_left_end, bridge_shimmer_t, &left);
			param_vex3d(&bridge_right_start, &bridge_right_end, bridge_shimmer_t, &right);

			draw_3D_line(&left, &right, context, clip);
		}
	}
#endif
}








//======================================================================================












void render_cube_wireframe(Cube* c, X3D_RenderContext* context, Polygon2D* clip, short id) {
	Vex3D rot[8];
	int i, d;
	Vex3D ncam_pos = {-context->cam.pos.x, -context->cam.pos.y, -context->cam.pos.z};

	ADDR(c);
	ADDR(context);
	ADDR(clip);

	cube_id = id;

	//if(id != 0 && id != 1)
	//	return;

	//printf("Visit %d\n", id);

	if(id == 5) {
		//print_polygon2d(clip);
	}

	for(i = 0; i < 8; i++) {
		Vex3D temp;

		// Translate the point to the opposite of the camera position
		add_vex3d(&c->v[i], &ncam_pos, &temp);

		// Rotate the point around the origin
		rotate_vex3d(&temp, &context->cam.mat, &rot[i]);

		//project_vex3d(context, &rot[i], &screen[i]);
	}

	union {
		Polygon3D poly3D;
		Polygon2D out_2d;
	} set_a;

	union {
		Polygon3D poly_out;
		Polygon2D dest;
	} set_b;

	for(i = 0; i < 6; i++) {
		cube_get_face(c->v, i, set_a.poly3D.v);

		set_a.poly3D.total_v = 4;

		//=========================================

		// Which edges of the current face we need to draw
		unsigned char draw_edges = 0;


		// If the angle between the view directions and the polygon normal < 45 deg,
		// we're going to assume that the polygon isn't visible
	#if 1
		if(dot_product(&c->normal[i], &context->cam.dir) > 23170)
			continue;
	#endif

		// If we're on the wrong side of the plane, it must be invisible (backface culling)
		short dist = dist_to_plane(&c->normal[i], &context->cam.pos, &c->v[cube_vertex_tab[i][0]]);

	#if 1
		if(dist > 0)
			continue;
	#endif

		for(d = 0; d < 4; d++) {
			short a = cube_vertex_tab[i][d];
			short b = cube_vertex_tab[i][d + 1];

			draw_edges = (draw_edges >> 1) | ((unsigned short)((c->edge_bits & (1 << edge_table[a][b])) == 0) << 3);
			c->edge_bits |= (1 << edge_table[a][b]);
		}

		draw_edges = 0b1111;

	#if 1
		// If none of the edges need to be drawn and this isn't a portal, we can skip clipping it
		if(draw_edges == 0 && c->cube[i] == -1)
			continue;
	#endif

		// Now that we know which edges need to be drawn, copy it over to the 3D polygon
		for(d = 0; d< set_a.poly3D.total_v; d++) {
			set_a.poly3D.draw[d] = draw_edges & 1;
			draw_edges >>= 1;
		}

		//=========================================

		Polygon2D* res = NULL;

		char draw_face = clip_polygon_to_frustum(&set_a.poly3D, &context->frustum, &set_b.poly_out);

		if(draw_face) {



			for(d = 0; d < set_b.poly_out.total_v; d++) {
				Vex3D temp;
				add_vex3d(&set_b.poly_out.v[d], &ncam_pos, &temp);
				rotate_vex3d(&temp, &context->cam.mat, &set_b.poly_out.v[d]);
			}

			project_polygon3d(&set_b.poly_out, context, &set_a.out_2d);

			for(d = 0; d < set_b.poly_out.total_v; d++)
				set_a.out_2d.line[d].draw = set_b.poly_out.draw[d];

			res = &set_b.dest;

			clip_polygon(&set_a.out_2d, clip, &set_b.dest, 0);//c->cube[i] == -1);



			draw_polygon(res, context);
		}
	}
}

extern char render_method;

// Renders the level, starting from the cube the camera is currently in
void render_level(X3D_RenderContext* c) {
	ADDR(c);

	// Create the clipping region
	Vex2D screen_clip[] = {
		{
			0, 0
		},
		{
			LCD_WIDTH - 1, 0
		},
		{
			LCD_WIDTH - 1, LCD_HEIGHT - 1
		},
		{
			0, LCD_HEIGHT - 1
		}
	};

	max_recursion_depth = 0;
	recursion_depth = 0;

	Polygon2D clip_region;

	if(render_method == 0)
		make_polygon2d(screen_clip, 4, &clip_region);

#if 1
	cube_tab[c->cam.current_cube].edge_bits = cube_tab[c->cam.current_cube].invisible_edges | (1 << 15);
	cube_tab[c->cam.current_cube].last_frame = c->frame;
	cube_tab[c->cam.current_cube].render_bits = 0;
#endif

	short old_y = c->cam.pos.y;

	// Move the camera up to eye level for rendering... not that this method
	// is buggy and should be replaced
	c->cam.pos.y -= PLAYER_HEIGHT;

	if(render_method == 0)
		render_cube(&cube_tab[c->cam.current_cube], c, &clip_region, c->cam.current_cube);
	else
		render_cube_clip3D(&cube_tab[c->cam.current_cube], c, &c->frustum, c->cam.current_cube);

	c->cam.pos.y = old_y;
}

// Passes information to another cube about which edges have already been drawn
// This prevents the 4x overdraw problem
//
// Returns whether the cube should be drawn or not
char cube_pass_edges(X3D_RenderContext* c, Cube* to, short face) {
	ADDR(c);
	ADDR(to);

	if(c->frame != to->last_frame) {
		to->edge_bits = to->invisible_edges;
		to->last_frame = c->frame;
		to->render_bits = 0;
	}

	unsigned char should_draw = !(to->render_bits & (1 << face));

	to->edge_bits |= edge_face_table[face];

	// Mark that we've drawn this cube through the face connecting
	// the parent cube
	to->render_bits |= (1 << face);

	return should_draw;
}


// Sets the position of the camera and updates the plane distances
// of the viewing frustum
void set_cam_pos(X3D_RenderContext* c, short x, short y, short z) {
	c->cam.pos = (Vex3D){x, y, z};

	calculate_frustum_plane_distances(c);

}

// Sets the angle of the camera and updates the planes of the view
// frustum
void set_cam_angle(X3D_RenderContext* c, unsigned char x, unsigned char y, unsigned char z) {
	c->cam.angle = (Vex3Ds){x, y, z};

	// Construct the new rotation matrix
	construct_mat3x3(&c->cam.angle, &c->cam.mat);

	// Get the direction the camera is facing
	// This exploits the fact that the first column of the rotation matrix turns
	// out to be the direction the camera is facing
	c->cam.dir.x = c->cam.mat[2][0];
	c->cam.dir.y = c->cam.mat[2][1];
	c->cam.dir.z = c->cam.mat[2][2];

	c->cam.straif_dir.x = c->cam.mat[0][0];
	c->cam.straif_dir.y = c->cam.mat[0][1];
	c->cam.straif_dir.z = c->cam.mat[0][2];

	// Update the view frustum
	calculate_frustum_rotated_normals(c);
	//calculate_frustum_plane_distances(c);

	set_cam_pos(c, c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);


}

// Builds the edge table, which allows us to determine which face has what
// edges
void build_edge_table() {
	int i, d;

	short id = 0;

	for(i = 0; i < 8; i++)
		for(d = 0; d < 8; d++)
			edge_table[i][d] = -1;

	// There's probably a better way to do this... oh well
	for(i = 0; i < 6; i++) {
		unsigned short bit = 0;

		for(d = 0; d < 4; d++) {
			short a = cube_vertex_tab[i][d];
			short b = cube_vertex_tab[i][d + 1];

			if(edge_table[a][b] == -1) {
				bit |= ((unsigned short)1 << id);

				edge_vertex_table[id][0] = a;
				edge_vertex_table[id][1] = b;

				edge_table[a][b] = id;
				edge_table[b][a] = id++;
			}
			else {
				bit |= ((unsigned short)1 << edge_table[a][b]);
			}
		}

		edge_face_table[i] = bit;
	}
}


// Initializes the renderer
void init_render() {
	build_edge_table();
}

// Returns whether the point is inside the cube or not
// Fail plane will contain the id of the plane it fails against
char point_in_cube(int id, Vex3D* point, char* fail_plane) {
	ADDR(point);


	Cube* c = &cube_tab[id];
	int i;

	for(i = 0; i < 6; i++) {
		Vex3D normal = c->normal[i];
		Vex3D p = c->v[cube_vertex_tab[i][0]];

		if(normal.x || normal.y || normal.z) {
			short dist = dist_to_plane(&normal, point, &p);

			if(c->cube[i] == CUBE_NONE) {
				if(dist > -DIST_TO_NEAR_PLANE) {
					*fail_plane = i;
					return 0;
				}
			}
			else if(dist > 0) {
				*fail_plane = i;

				return 0;
			}
		}
	}

	return 1;
}

// Attempts to move the camera and update which cube the camera is in
// Move mask determines which axes we're allowed to move along
void attempt_move_cam(X3D_RenderContext* c, Vex3DL* dir, short speed, unsigned char move_mask) {
	char fail_plane;
	Vex3DL add = {0, 0, 0};


	if(move_mask & 1)
		add.x = ((long)dir->x * speed);

	if(move_mask & 2)
		add.y = ((long)dir->y * speed);

	if(move_mask & 4)
		add.z = ((long)dir->z * speed);


	Vex3DL new_pos_long = {c->cam.pos_long.x + add.x, c->cam.pos_long.y + add.y, c->cam.pos_long.z + add.z};
	Vex3D new_pos = {new_pos_long.x >> NORMAL_BITS, new_pos_long.y >> NORMAL_BITS, new_pos_long.z >> NORMAL_BITS};

	int i;

	char first = 1;

	for(i = 0; i < 15; i++) {
		if(point_in_cube(c->cam.current_cube, &new_pos, &fail_plane)) {
			c->cam.pos = new_pos;

			set_cam_pos(c, c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);
			c->cam.pos_long = new_pos_long;
			return;
		}
		else {
			if(cube_tab[c->cam.current_cube].cube[(short)fail_plane] != -1) {
				c->cam.pos = new_pos;
				c->cam.current_cube = cube_get_child(&cube_tab[c->cam.current_cube], fail_plane);

				set_cam_pos(c, c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);

				c->cam.pos_long = new_pos_long;
				return;
			}
			else if(fail_plane == PLANE_BOTTOM) {
				c->cam.on_ground = 1;
			}
		}

		if(fail_plane == PLANE_BOTTOM) {
			// Calculate the equation of the plane
			Cube* cube = &cube_tab[c->cam.current_cube];
			Vex3D* normal = &cube->normal[PLANE_BOTTOM];

			short d = dot_product(normal, &cube->v[cube_vertex_tab[PLANE_BOTTOM][0]]);

			// Evalutate the y coordinate of the plane
			long top = ((-(long)normal->x * c->cam.pos.x - (long)normal->z * c->cam.pos.z) >> NORMAL_BITS) + d;

			short y =  ((long)top << NORMAL_BITS) / normal->y;

			short add = -DIST_TO_NEAR_PLANE - dist_to_plane(&cube->normal[PLANE_BOTTOM], &new_pos, &cube->v[cube_vertex_tab[PLANE_BOTTOM][0]]);

			if(add >= 0) {
				add = -1;
			}

			new_pos.y += add;

			new_pos_long.y = (long)new_pos.y << NORMAL_BITS;

		}
	}
}





















