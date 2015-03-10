// C Source File
// Created 3/7/2015; 6:28:25 PM

#include "geo.h"

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
	
	// Calculate the unrotated planes of the view frustum
	calculate_frustum_plane_normals(c);
}

// Draws a polygon
void draw_polygon(Polygon2D* p, RenderContext* context) {
	int i, next;
	
	for(i = 0; i < p->total_v; i++) {
		next = (i + 1) % p->total_v;
		draw_clip_line(p->v[i].x, p->v[i].y, p->v[next].x, p->v[next].y, context->screen);
	}
}

// Renders a single cube in writeframe
// Note: make sure the cube isn't off the screen at all!
void render_cube(Cube* c, RenderContext* context) {
	Vex3D rot[8];
	Vex2D screen[8];
	int i;
	Vex3D ncam_pos = {-context->cam.pos.x, -context->cam.pos.y, -context->cam.pos.z}; 
	
	for(i = 0; i < 8; i++) {
		Vex3D temp;
		
		// Translate the point to the opposite of the camera position
		add_vex3d(&c->v[i], &ncam_pos, &temp);
		
		// Rotate the point around the origin
		rotate_vex3d(&temp, &context->cam.mat, &rot[i]);
		
		//project_vex3d(context, &rot[i], &screen[i]);
	}
	
	Polygon3D poly3D, poly_out;
	Polygon2D poly2D;
	
	for(i = 0; i < 1; i++) {
		cube_get_face(rot, i, poly3D.v);
		poly3D.total_v = 4;
		
		clip_polygon_to_plane(&poly3D, &context->frustum.p[4], &poly_out);
		//print_polygon(&poly3D);
		//ngetchx();
		project_polygon3d(&poly_out, context, &poly2D);
		draw_polygon(&poly2D, context);
	}
	
	
	
	
	
#if 0
	for(i = 0; i < 3; i++) {
		DrawLine(screen[i].x, screen[i].y, screen[i + 1].x, screen[i + 1].y, A_NORMAL);
		DrawLine(screen[i + 4].x, screen[i + 4].y, screen[i + 4 + 1].x, screen[i + 4 + 1].y, A_NORMAL);
	}
	
	DrawLine(screen[0].x, screen[0].y, screen[3].x, screen[3].y, A_NORMAL);
	DrawLine(screen[4].x, screen[4].y, screen[7].x, screen[7].y, A_NORMAL);
	
	for(i = 0; i < 4; i++) {
		DrawLine(screen[i].x, screen[i].y, screen[i + 4].x, screen[i + 4].y, A_NORMAL);
	}
#endif	
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




























