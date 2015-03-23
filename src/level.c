// C Source File
// Created 3/12/2015; 10:40:21 AM

#include "geo.h"

#include <tigcclib.h>

// All of the cubes in the level
Cube *cube_tab; //[20];
short total_cubes;

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
	
	construct_cube(s, s, s, 2 * s, -2 * s, s, &cube_angle, &cube_tab[6]);
	construct_cube(s, s, s, 2 * s, -2 * s, 2 * s, &cube_angle, &cube_tab[7]);
	
	// Connect here
	construct_cube(s, s, s, -2 * s, 0, s, &cube_angle, &cube_tab[8]);
	
	construct_cube(s, s, s, -2 * s, -s, s, &cube_angle, &cube_tab[9]);
	
	construct_cube(s, s, s, 2 * s, -2 * s, 3 * s, &cube_angle, &cube_tab[10]);
	
	construct_cube(s, s, s, 2 * s, -1 * s, 3 * s, &cube_angle, &cube_tab[11]);
	
	construct_cube(s, s, s, 1 * s, -1 * s, 3 * s, &cube_angle, &cube_tab[12]);
	
	construct_cube(s, s, s, 0, -1 * s, 3 * s, &cube_angle, &cube_tab[13]);
	
	construct_cube(s, s, s, 0, 0, 3 * s, &cube_angle, &cube_tab[14]);
	
	construct_cube(s, s, s, -s, 0, 3 * s, &cube_angle, &cube_tab[15]);
	
	construct_cube(s, s, s, -2 * s, 0, 3 * s, &cube_angle, &cube_tab[16]);
	
	construct_cube(s, s, s, -2 * s, 0, 2 * s, &cube_angle, &cube_tab[17]);
	
	
	//cube_tab[0].cube[PLANE_BACK] = 1;
	//cube_tab[1].cube[PLANE_LEFT] = 2;
	//cube_tab[1].cube[PLANE_TOP] = 3;
	
	connect_cube(0, 1, PLANE_BACK);
	connect_cube(1, 2, PLANE_LEFT);
	connect_cube(1, 3, PLANE_TOP);
	
	connect_cube(3, 4, PLANE_TOP);
	connect_cube(4, 5, PLANE_RIGHT);
	
	connect_cube(5, 6, PLANE_RIGHT);
	
	connect_cube(6, 7, PLANE_BACK);
	
	connect_cube(2, 8, PLANE_LEFT);
	
	connect_cube(8, 9, PLANE_TOP);
	
	connect_cube(7, 10, PLANE_BACK);
	connect_cube(10, 11, PLANE_BOTTOM);
	
	connect_cube(11, 12, PLANE_LEFT);
	
	connect_cube(12, 13, PLANE_LEFT);
	
	connect_cube(13, 14, PLANE_BOTTOM);
	
	connect_cube(14, 15, PLANE_LEFT);
	
	connect_cube(15, 16, PLANE_LEFT);
	
	connect_cube(16, 17, PLANE_FRONT);
	
	connect_cube(8, 17, PLANE_BACK);
	
	level_remove_redundant_edges(18);
}

// Connects two cubes together so the renderer knows they share a face
void connect_cube(short parent, short child, short plane) {
	cube_tab[parent].cube[plane] = child;
	cube_tab[child].cube[get_opposite_face(plane)] = parent;
}

// Examines the edges of a cube and the cubes that are connected to it -
// if the plane normals of the cube with its neighbors are the same, it makes
// them as invisible.
void cube_remove_redundant_edges(Cube* c) {
	int i, d;
	
	// Initially assume that all edges are visible
	unsigned short invisible_edges = 0;
	
	for(i = 0; i < 6; i++) {
		// If there's a cube connected to this face...
		if(c->cube[i] != CUBE_NONE) {
			short opposite_face = get_opposite_face(i);
			Cube* c2 = &cube_tab[c->cube[i]];
			
			// Consider every face except the one that connects them and its opposite
			// i.e. if a cube A is connected to cube B by the back plane, the edges that
			// make up the portal are shared between the top, bottom, left, and right plane.
			// So, we should consider every plane except the front and back plane.
			// Also, we need to not remove edges from 
			
			for(d = 0; d < 6; d++) {
				if(d != i && d != opposite_face && c2->cube[d] == CUBE_NONE && c->cube[d] == CUBE_NONE) {
					// If the normals between the corresponding faces are roughly the same,
					// remove the edge between them
					if(abs(c->normal[d].x - c2->normal[d].x) < 5 && abs(c->normal[d].y - c2->normal[d].y) < 5 &&
						abs(c->normal[d].z - c2->normal[d].z) < 5) {
						
						// The edges that compose the portal
						unsigned short portal_edges = edge_face_table[i];
						
						// The edges that compose the face we're checking
						unsigned short face_edges = edge_face_table[d];
						
						// Find the one edge they have in common and mark it invisible
						invisible_edges |= (portal_edges & face_edges);
					}
				}
			}
			
		}
	}
	
	c->invisible_edges = invisible_edges;
}

// Removes redundant edges for the entire level
// See cube_remove_redundant_edges() for rationale
void level_remove_redundant_edges(short total_cubes) {
	int i;
	
	for(i =  0; i < total_cubes; i++) {
		cube_remove_redundant_edges(&cube_tab[i]);
	}
}

enum {
	VEX_LFB,
	VEX_LBB,
	VEX_RBB,
	VEX_RFB,
	
	VEX_LFT,
	VEX_LBT,
	VEX_RBT,
	VEX_RFT
};


char load_level(const char* name) {
	FILE* file = fopen(name, "rb");
	
	if(!file) {
		printf("Couldn't open file\n");
		return 0;
	}
	
	char n[] = {0, 0, 0, 0, 0, 0, 0};
	
	fread(n, 1, 6, file);
	
	if(strcmp("X3DXDL", n) != 0) {
		printf("Error: not a descent level v0\n");
		return 0;
	}
	
	if(fgetc(file) != 0) {
		printf("Wrong version\n");
		return 0;
	}
	
	short cubes;
	fread(&cubes, 2, 1, file);
	
	printf("Loading level...\n");
	printf("Total cubes: %d\n", cubes);
	total_cubes = cubes;
	
	cube_tab = malloc(sizeof(Cube) * cubes);
	
	int i;
	int d;
	
/*
	enum {
		VEX_UBL,
		VEX_UTL,
		VEX_UTR,
		VEX_UBR,
		VEX_DBL,
		VEX_DTL,
		VEX_DTR,
		VEX_DBR
	};

	c->v[VEX_UBL] =  (Vex3D){-x, y, -z};	LFB
	c->v[VEX_UTL] =  (Vex3D){-x, y, z};		LBB
	c->v[VEX_UTR] =  (Vex3D){x, y, z};		RBB
	c->v[VEX_UBR] =  (Vex3D){x, y, -z};		RFB
	
	c->v[VEX_DBL] =  (Vex3D){-x, -y, -z};   LFT
	c->v[VEX_DTL] =  (Vex3D){-x, -y, z};	LBT
	c->v[VEX_DTR] =  (Vex3D){x, -y, z};		RBT
	c->v[VEX_DBR] =  (Vex3D){x, -y, -z};	RFT
	
	=================================================
	
	LFB
	LBB
	RBB
	RFB
	
	LFT
	LBT
	RBT
	RFT
	
	0 - left, front, top
	1 - left, front, bottom
	2 - right, front, bottom
	3 - right, front, top
	4 - left, back, top
	5 - left, back, bottom
	6 - right, back, bottom
	7 - right, back, top
	
	
*/

	// Descent stores the verticies in a different order; this tables maps their
	// order to X3D's order
	short vex_tab[] = {
		VEX_LFT,
		VEX_LFB,
		VEX_RFB,
		VEX_RFT,
		VEX_LBT,
		VEX_LBB,
		VEX_RBB,
		VEX_RBT
		
		
		
		
		/*VEX_RFT,
		VEX_RBT,
		VEX_LBT,
		VEX_LFT,
		VEX_RFB,
		VEX_RBB,
		VEX_LBB,
		VEX_LFB*/
		
		
		
		
		
		/*
		
		VEX_LFT,
		VEX_LFB,
		VEX_RFB,
		VEX_RFT,
		VEX_LBT,
		VEX_LBB,
		VEX_RBB,
		VEX_RBT
		
	*/
	};

	// Unfortunately, Descent also stores their faces in a different order too
	// TODO: this conversion should be done in the level editor
	short cube_plane_tab[] = {
		PLANE_LEFT,
		PLANE_TOP,
		PLANE_RIGHT,
		PLANE_BOTTOM,
		PLANE_BACK,
		PLANE_FRONT
	};


	Vex3D v;
	short cube;
	
	for(i = 0; i < cubes; i++) {
		Cube* c = &cube_tab[i];
		
		// Read in the verticies
		for(d = 0; d < 8; d++) {
			fread(&v.x, 2, 1, file);
			fread(&v.y, 2, 1, file);
			fread(&v.z, 2, 1, file);
			
			v.x *= 10;
			v.y *= 10;
			v.z *= 10;
			
			v.y = -v.y;
			v.x = -v.x;
			
			c->v[vex_tab[d]] = v;
			
			if(i == 0) {
				printf("%d: ", d);
				print_vex3d(&v);
			}
			//ngetchx();
		}
		
		// Calculate the plane normals
		Vex3D da, db;
		Vex3D ap, bp, cp;
		
		for(d = 0; d < 6; d++) {
			ap = c->v[cube_vertex_tab[d][0]];
			bp = c->v[cube_vertex_tab[d][1]];
			cp = c->v[cube_vertex_tab[d][2]];
			
			sub_vex3d(&ap, &bp, &da);
			sub_vex3d(&cp, &bp, &db);
			
			cross_product(&db, &da, &c->normal[d]);
			
			//if(i == 0)
			//	print_vex3d(&c->normal[d]);
			//ngetchx();
		}
		
		// Read in the cubes that are connected to it
		for(d = 0; d < 6; d++) {
			fread(&cube, 2, 1, file);
			
			//printf("Cube: %d\n", cube);
			//ngetchx();
			
			if(cube == -2)
				cube = -1;
			
			c->cube[cube_plane_tab[d]] = cube;
			
			if(i == 0) {
				//printf("Cube %d: %d\n", d, c->cube[cube_plane_tab[d]]);
				//print_vex3d(&c->normal[d]);
				
				if(d == 5) {
					int k;
					
					for(k = 0; k < 6; k++) {
						//print_vex3d(&c->v[cube_vertex_tab[cube_plane_tab[d]][k]]);
						//print_vex3d(&c->normal[k]);
					}
				}
			}
			
		}
		
		c->invisible_edges = 0;
	}
	
	printf("Removing redundant edges\n");
	//level_remove_redundant_edges(cubes);
	
	
	printf("Done loading level\n");
	ngetchx();
	
	fclose(file);
	
	return 1;
}














