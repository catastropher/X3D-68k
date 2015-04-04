// C Source File
// Created 3/8/2015; 8:07:15 AM

#include <tigcclib.h>

#include "geo.h"
#include "console.h"

// Prints out a 3D vertex/vector
void print_vex3d(Vex3D* v) {
	printf("Vex3D{%d, %d, %d}\n", v->x, v->y, v->z);
}

// Prints out a 2D vertex/vector
void print_vex2d(Vex2D* v) {
	printf("Vex2D{%d, %d}\n", v->x, v->y);
}

// Prints out a plane
void print_plane(Plane* p) {
	printf("======Plane======\nNormal: ");
	print_vex3d(&p->normal);
	printf("D: %d\n", p->d);
}

// Prints out the points of a 3D polygon
void print_polygon(Polygon* p) {
	int i;
	
	printf("============\n");
	
	for(i = 0; i < p->total_v; i++) {
		printf("%d: {%d, %d, %d}\n", i, p->v[i].x, p->v[i].y, p->v[i].z);
	}
	
	printf("============\n");
}

// Prints out the points of a 2D polygon
void print_polygon2d(Polygon2D* p) {
	int i;
	
	printf("============\n");
	
	for(i = 0; i < p->total_v; i++) {
		printf("Draw: %d ", p->line[i].draw);
		print_vex2d(&p->p[i].v);
	}
	
	printf("============\n");
}

// Prints out a frustum
void print_frustum(Frustum* f) {
	int i;
	
	for(i = 0; i < f->total_p; i++) {
		print_plane(&f->p[i]);
		ngetchx();
	}
}