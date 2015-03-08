// C Source File
// Created 3/5/2015; 9:25:21 AM

#include "geo.h"
#include "math.h"
#include "screen.h"

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

// Projects a 3D point onto a 2D surface i.e. a RenderContext
// Note: make sure the z component of src is not 0 or you will get division
// by 0!
void project_vex3d(RenderContext* rc, Vex3D* src, Vex2D* dest) {
	short inv_z = ((long)rc->scale << NORMAL_BITS) / src->z;
	
	dest->x = ((long)src->x * inv_z) >> NORMAL_BITS;
	dest->y = ((long)src->y * inv_z) >> NORMAL_BITS;
}

// Subtracts two 3D vectors: dest = a - b
void sub_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest) {
	dest->x = a->x - b->x;
	dest->y = a->y - b->y;
	dest->z = a->z - b->z;
}

// Construct a plane given 3 points on the plane
void construct_plane(Vex3D* a, Vex3D* b, Vex3D* c, Plane* dest) {
	Vex3D v1, v2;
	
	// Calculate the normal of the plane
	sub_vex3d(a, b, &v1);
	sub_vex3d(c, b, &v2);
	
	cross_product(&v1, &v2, &dest->normal);
	
	// D = AX + BY + CZ
	dest->d = dot_product(&dest->normal, a);
}

// Multiplies two 3x3 matricies i.e. concatenates them
void mul_mat3x3(Mat3x3* a, Mat3x3* b, Mat3x3* res) {
	int i, j, k;
	
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			(*res)[i][j] = 0;

	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			for(k = 0; k < 3; k++) {
				(*res)[i][j] += FIXMUL15((*a)[i][k], (*b)[k][j]);
			}
		}
	}
}

// Constructs a 3D rotation matrix with the given angles
// TODO: rewrite this to not use matrix multiplication
void construct_mat3x3(Vex3Ds *angle, Mat3x3 *dest) {
	short sin_y = sinfp(angle->y);
	short cos_y = cosfp(angle->y);
		
	short sin_x = sinfp(angle->x);
	short cos_x = cosfp(angle->x);
	
	Mat3x3 mat_y = {
		{ cos_y, 0, sin_y}, 
		{ 0, 32767, 0}, 
		{ -sin_y, 0, cos_y}
	};
	
	Mat3x3 mat_x = {
		{32767, 0, 0},
		{0, cos_x, -sin_x},
		{0, sin_x, cos_x},
	};
	
	Mat3x3 mul_res;
	
	mul_mat3x3(&mat_x, &mat_y, &mul_res);
	
	memcpy(dest, &mul_res, sizeof(Mat3x3));
}

// Given the input angle in DEG256, this returns the fixed-point sine of it
// The number is in 0:15 format
inline short sinfp(unsigned char angle) {
	return sintab[angle];
}

// Given the input angle in DEG256, this returns the fixed-point cosine of it
// The number is in 0:15 format
inline short cosfp(unsigned char angle) {
	// We exploit the fact that cos(x) = sin(90 - x)
	return sinfp(ANG_90 - angle);
}

// Given the input angle in DEG256, this returns the fixed-point cosine of it
// The number is in 8:8 format
inline short tanfp(unsigned char angle) {
	// Precent division by 0
	if(angle == ANG_90 || angle == ANG_180)
		return VERTICAL_LINE_SLOPE;
	
	return ((long)sinfp(angle) << 8) / cosfp(angle);
}

