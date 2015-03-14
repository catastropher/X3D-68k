// C Source File
// Created 3/12/2015; 10:40:21 AM

#include "geo.h"

#include <tigcclib.h>

// All of the cubes in the level
Cube cube_tab[10];

// Creates a simple level for testing purposes
void create_test_level() {
	Vex3Ds cube_angle = {0, 0, 0};
	
	int s = 400;
	
	construct_cube(s, s, s, 0, 0, 0, &cube_angle, &cube_tab[0]);
	construct_cube(s, s, s, 0, 0, s, &cube_angle, &cube_tab[1]);
	
	construct_cube(s, s, s, 0, -s, s, &cube_angle, &cube_tab[3]);
	
	construct_cube(s, s, s, -s, 0, s, &cube_angle, &cube_tab[2]);
	
	
	construct_cube(s, s, s, 0, -2 * s, s, &cube_angle, &cube_tab[4]);
	construct_cube(s, s, s, s, -2 * s, s, &cube_angle, &cube_tab[5]);
	
	
	//cube_tab[0].cube[PLANE_BACK] = 1;
	//cube_tab[1].cube[PLANE_LEFT] = 2;
	//cube_tab[1].cube[PLANE_TOP] = 3;
	
	connect_cube(0, 1, PLANE_BACK);
	connect_cube(1, 2, PLANE_LEFT);
	connect_cube(1, 3, PLANE_TOP);
	
	connect_cube(3, 4, PLANE_TOP);
	connect_cube(4, 5, PLANE_RIGHT);
}

// Connects two cubes together so the renderer knows they share a face
void connect_cube(short parent, short child, short plane) {
	cube_tab[parent].cube[plane] = child;
	cube_tab[child].cube[get_opposite_face(plane)] = parent;
}

#if 0
void cube_pass_edges(CBuffer* buf, Cube* to, short face) {
	if(buf->frame != to->last_frame) {
		to->edge_bits = 0;
		to->last_frame = buf->frame;
	}
	
	
	to->edge_bits |= edge_face_table[get_opposite_face(face)];
}
#endif