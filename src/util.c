// C Source File
// Created 3/8/2015; 8:07:15 AM

#include "geo.h"

#include <tigcclib.h>

// Prints out a 3D vertex/vector
void print_vex3d(Vex3D* v) {
	printf("Vex3D{%d, %d, %d}\n", v->x, v->y, v->z);
}

// Prints out a plane
void print_plane(Plane* p) {
	printf("======Plane======\nNormal: ");
	print_vex3d(&p->normal);
	printf("D: %d\n", p->d);
}