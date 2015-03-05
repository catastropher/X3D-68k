// C Source File
// Created 3/5/2015; 9:25:21 AM

#include "geo.h"
#include "math.h"

#include <tigcclib.h>

// Calculates the dot product of two vectors, which can be interpreted
// as the cosine of the angle between them
short dot_product(Vex3D* a, Vex3D* b) {
	long prod = (long)a->x * b->x + (long)a->y * b->y + (long)a->z * b->z;
	
	return prod >> NORMAL_BITS;
}

// Calculates the cross product of two vectors. This creates a vector that
// is perpindicular to both vectors
void cross_product(Vex3D* a, Vex3D* b, Vex3D* dest) {
	dest->x = ((long)a->y * b->z - (long)a->z * b->y) >> NORMAL_BITS;
	dest->y = ((long)a->z * b->x - (long)a->x * b->z) >> NORMAL_BITS;
	dest->z = ((long)a->x * b->y - (long)a->y * b->x) >> NORMAL_BITS;
}