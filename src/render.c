// C Source File
// Created 3/7/2015; 6:28:25 PM

#include "geo.h"
#include "extgraph.h"

#include <tigcclib.h>

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


// Initializes a render context
void init_render_context(short w, short h, short x, short y, unsigned char fov, RenderContext* c) {
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

// Draws a polygon
void draw_polygon(Polygon2D* p, RenderContext* context) {
	int i, next;
	
	for(i = 0; i < p->total_v; i++) {
		//next = (i + 1) % p->total_v;
		next = i + 1;
		
		if(next == p->total_v)
			next = 0;
		
		
		//draw_clip_line(p->v[i].x, p->v[i].y, p->v[next].x, p->v[next].y, context->screen);
		FastLine_Draw_R(context->screen, p->p[i].v.x, p->p[i].v.y, p->p[next].v.x, p->p[next].v.y);
	}
}

// Renders a single cube in writeframe
// Note: make sure the cube isn't off the screen at all!
void render_cube(Cube* c, RenderContext* context, Polygon2D* clip) {
	Vex3D rot[8];
	Vex2D screen[8];
	int i, d;
	Vex3D ncam_pos = {-context->cam.pos.x, -context->cam.pos.y, -context->cam.pos.z}; 
	
	// Make this cube as visited
	c->last_frame = context->frame;
	
	for(i = 0; i < 8; i++) {
		Vex3D temp;
		
		// Translate the point to the opposite of the camera position
		add_vex3d(&c->v[i], &ncam_pos, &temp);
		
		// Rotate the point around the origin
		rotate_vex3d(&temp, &context->cam.mat, &rot[i]);
		
		//project_vex3d(context, &rot[i], &screen[i]);
	}
	
	Polygon3D poly3D, poly_out, poly_out2;
	Polygon2D poly2D;
	
	for(i = 0; i < 6; i++) {
		cube_get_face(c->v, i, poly3D.v);
		poly3D.total_v = 4;
		
		//clip_polygon_to_plane(&poly3D, &context->frustum.p[0], &poly_out2);
		//clip_polygon_to_plane(&poly_out2, &context->frustum.p[4], &poly_out);
		//print_polygon(&poly3D);
		//ngetchx();
		
		
	#if 0
		if(clip_polygon_to_frustum(&poly3D, &context->frustum, &poly_out)) {
			for(d = 0; d < poly_out.total_v; d++) {
				Vex3D temp;
				add_vex3d(&poly_out.v[d], &ncam_pos, &temp);
				rotate_vex3d(&temp, &context->cam.mat, &poly_out.v[d]);
			}
				
			
			project_polygon3d(&poly_out, context, &poly2D);
			draw_polygon(&poly2D, context);
		}
	#else
	
		for(d = 0; d < poly3D.total_v; d++) {
			Vex3D temp;
			add_vex3d(&poly3D.v[d], &ncam_pos, &temp);
			rotate_vex3d(&temp, &context->cam.mat, &poly_out.v[d]);
		}
		
		//clip_polygon(Polygon2D* p, Polygon2D* clip, Polygon2D* temp_a, Polygon2D* temp_b);
		
		poly_out.total_v = poly3D.total_v;
		
		Polygon2D out_2d;
		
		project_polygon3d(&poly_out, context, &out_2d);
		Polygon2D temp_a, temp_b;
		
		
		Polygon2D* res = clip_polygon(&out_2d, clip, &temp_a, &temp_b);
		
		draw_polygon(res, context);
		
	#endif
	
		
		// If there's a cube connected to this face
		if(c->cube[i] != -1) {
			Cube* next_cube = &cube_tab[c->cube[i]];
			
			// Make sure we haven't rendered it yet
			if(next_cube->last_frame != context->frame)
				render_cube(next_cube, context, clip);
		}
	}
}

// Renders the level, starting from the cube the camera is currently in
void render_level(RenderContext* c) {
	// Create the clipping region
	Vex2D screen_clip[] = {
		{
			10, 10
		},
		{
			230, 10
		},
		{
			230, 120
		},
		{
			10, 120
		}
	};
	
	Polygon2D clip_region;
	
	make_polygon2d(screen_clip, 4, &clip_region);
	
	render_cube(&cube_tab[0], c, &clip_region);
	
	draw_polygon(&clip_region, c);
}


// Sets the position of the camera and updates the plane distances
// of the viewing frustum
void set_cam_pos(RenderContext* c, short x, short y, short z) {
	c->cam.pos = (Vex3D){x, y, z};
	
	calculate_frustum_plane_distances(c);
	
	// Calculate the distance from the origin to the camera
	//c->cam.dist_from_origin = get_vex3d_magnitude(&c->cam.pos);
	
	// Update the plane equations
	int i;
	
	//for(i = 0; i < c->frustum.total_p; i++)
	//	c->frustum.p[i].d = c->frustum_unrotated.p[i].d + c->cam.dist_from_origin;
	
}

// Sets the angle of the camera and updates the planes of the view
// frustum
void set_cam_angle(RenderContext* c, unsigned char x, unsigned char y, unsigned char z) {
	c->cam.angle = (Vex3Ds){x, y, z};
	
	// Construct the new rotation matrix
	construct_mat3x3(&c->cam.angle, &c->cam.mat);
	
	// Get the direction the camera is facing
	// This exploits the fact that the first column of the rotation matrix turns
	// out to be the direction the camera is facing
	c->cam.dir.x = c->cam.mat[2][0];
	c->cam.dir.y = c->cam.mat[2][1];
	c->cam.dir.z = c->cam.mat[2][2];
	
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
	
#if 0
	for(i = 0; i < 6; i++) {
		unsigned short b = edge_face_table[i];
		
		for(d = 0; d < 12; d++) {
			if(b & (1 << 11)) {
				printf("1");
			}
			else {
				printf("0");
			}
			
			b <<= 1;
		}
		printf("\n");
	}
	
	for(i = 0; i < 8; i++) {
		for(d = 0; d < 8; d++) {
			printf("%d ", edge_table[i][d]);
		}
		printf("\n");
	}
	
	if(id != 12)
		error("ERR");
#endif
}


// Initializes the renderer
void init_render() {
	build_edge_table();
}























