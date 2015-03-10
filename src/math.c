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
// is perpendicular to both vectors
// Note: this routine will normalize the result
void cross_product(Vex3D* a, Vex3D* b, Vex3D* dest) {
	dest->x = ((long)a->y * b->z - (long)a->z * b->y);
	dest->y = ((long)a->z * b->x - (long)a->x * b->z);
	dest->z = ((long)a->x * b->y - (long)a->y * b->x);
	
	//printf("Dest->z: %d\n", dest->z);
	
	normalize_vex3d(dest);
}

// Projects a 3D point onto a 2D surface i.e. a RenderContext
// Note: make sure the z component of src is not 0 or you will get division
// by 0!
void project_vex3d(RenderContext* rc, Vex3D* src, Vex2D* dest) {
	short inv_z = ((long)rc->dist << NORMAL_BITS) / src->z;
	
	//dest->x = (((long)src->x * inv_z) >> NORMAL_BITS) + rc->center_x;
	//dest->y = (((long)src->y * inv_z) >> NORMAL_BITS) + rc->center_y;
	
	
	dest->x = ((long)src->x * rc->dist) / src->z + rc->center_x;
	dest->y = ((long)src->y * rc->dist) / src->z + rc->center_y;
}

// Subtracts two 3D vectors: dest = a - b
void sub_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest) {
	dest->x = a->x - b->x;
	dest->y = a->y - b->y;
	dest->z = a->z - b->z;
}

// Adds two 3D vectors: dest = a + b
void add_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest) {
	dest->x = a->x + b->x;
	dest->y = a->y + b->y;
	dest->z = a->z + b->z;
}

short get_vex3d_magnitude(Vex3D* v) {
	return fastsqrt((long)v->x * v->x + (long)v->y * v->y + (long)v->z * v->z);
}

// Normalizes a 3D vector i.e. makes the length of the vector 1
// The result is in 0:15 format
void normalize_vex3d(Vex3D* v) {
	short len = fastsqrt((long)v->x * v->x + (long)v->y * v->y + (long)v->z * v->z) + 1;
	
	v->x = ((long)v->x << NORMAL_BITS) / len;
	v->y = ((long)v->y << NORMAL_BITS) / len;
	v->z = ((long)v->z << NORMAL_BITS) / len;
	
}

// Rotates a Vex3D around the origin
void rotate_vex3d(Vex3D* src, Mat3x3* mat, Vex3D* dest) {
	Vex3D_rot rot;
	asm_rotate_vex3d(src, mat, &rot);
	
	dest->x = rot.x;
	dest->y = rot.y;
	dest->z = rot.z;
}

// Construct a plane given 3 points on the plane
void construct_plane(Vex3D* a, Vex3D* b, Vex3D* c, Plane* dest) {
	Vex3D v1, v2;
	
	// Calculate the normal of the plane
	sub_vex3d(a, b, &v1);
	sub_vex3d(c, b, &v2);
	
	cross_product(&v1, &v2, &dest->normal);
	
	// D = -(AX + BY + CZ)
	dest->d = -dot_product(&dest->normal, a);
}

// Calculates the normals of the unrotated planes of the view frustum
void calculate_frustum_plane_normals(RenderContext* c) {
	short w = c->w;
	short h = c->h;
	
	Vex3D top_left = {-w / 2, -h / 2, c->dist};
	Vex3D top_right = {w / 2, -h / 2, c->dist};
	
	Vex3D bottom_left = {-w / 2, h / 2, c->dist};
	Vex3D bottom_right = {w / 2, h / 2, c->dist};
	
	Vex3D cam_pos = {0, 0, 0};
	
	// Top plane
	construct_plane(&cam_pos, &top_right, &top_left, &c->frustum_unrotated.p[0]);
	
	// Bottom plane
	construct_plane(&cam_pos, &bottom_left, &bottom_right, &c->frustum_unrotated.p[1]);
	
	// Left plane
	construct_plane(&cam_pos, &top_left, &bottom_left, &c->frustum_unrotated.p[2]);
	
	// Right plane
	construct_plane(&cam_pos, &bottom_right, &top_right, &c->frustum_unrotated.p[3]);
	
	// Near plane
	construct_plane(&bottom_right, &top_right, &top_left, &c->frustum_unrotated.p[4]);
	
	// Hack...
	c->frustum_unrotated.p[4].d = -DIST_TO_NEAR_PLANE;
	
	c->frustum.total_p = 5;
}

// Calculates the distance from each plane in the view frustum to the origin
void calculate_frustum_plane_distances(RenderContext* c) {
	int i;
	
	for(i = 0; i < c->frustum.total_p - 1; i++) {
		c->frustum.p[i].d = -dot_product(&c->frustum.p[i].normal, &c->cam.pos);
	}
	
	Vex3D input = {0, 0, DIST_TO_NEAR_PLANE * 4};
	Vex3D out;
	
	rotate_vex3d(&input, &c->cam.mat, &out);
	
	out.x >>= 2;
	out.y >>= 2;
	out.z >>= 2;
	
	//out.x = out.x;
	//out.y = -out.y;
	//out.z = -out.z;
	
	out.x += c->cam.pos.x;
	out.y += c->cam.pos.y;
	out.z += c->cam.pos.z;
	
	c->frustum.p[4].d = -dot_product(&c->frustum.p[4].normal, &out);
	
	printf("Out: %d\n", c->frustum.p[4].d);
	
}

// Calculates the rotated plane normals of the view frustum
void calculate_frustum_rotated_normals(RenderContext* c) {
	int i;
	
	for(i = 0; i < c->frustum.total_p; i++) {
		rotate_vex3d(&c->frustum_unrotated.p[i].normal, &c->cam.mat, &c->frustum.p[i].normal);
	}
	
	c->frustum.total_p = c->frustum_unrotated.total_p;
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
	// Prevent division by 0
	if(angle == ANG_90 || angle == ANG_180)
		return VERTICAL_LINE_SLOPE;
	
	return ((long)sinfp(angle) << 8) / cosfp(angle);
}

// Constructs a cube of size (x, y, z) at position (posx, posy, posz)
// rotated by the given angle
void construct_cube(short x, short y, short z, short posx, short posy, short posz, Vex3Ds* angle, Cube* c) {
	x /= 2;
	y /= 2;
	z /=2 ;

	c->v[VEX_UBL] =  (Vex3D){-x, y, -z};
	c->v[VEX_UTL] =  (Vex3D){-x, y, z};
	c->v[VEX_UTR] =  (Vex3D){x, y, z};
	c->v[VEX_UBR] =  (Vex3D){x, y, -z};
	
	c->v[VEX_DBL] =  (Vex3D){-x, -y, -z};
	c->v[VEX_DTL] =  (Vex3D){-x, -y, z};
	c->v[VEX_DTR] =  (Vex3D){x, -y, z};
	c->v[VEX_DBR] =  (Vex3D){x, -y, -z};
	
	short v = -32767;
	
	c->normal[PLANE_BOTTOM] = (Vex3D){0, v, 0};
	c->normal[PLANE_TOP] = (Vex3D){0, -v, 0};
	c->normal[PLANE_FRONT] = (Vex3D){0, 0, -v};
	c->normal[PLANE_BACK] = (Vex3D){0, 0, v};
	c->normal[PLANE_LEFT] = (Vex3D){-v, 0, 0};
	c->normal[PLANE_RIGHT] =(Vex3D){v, 0, 0};
	
	
	int i;
	
	for(i = 0; i < 6; i++)
		c->cube[i] = -1;
	
#if 0
	Mat3x3 mat, mat2;
	x3d_construct_mat3_old(angle, &mat);
	
	x3d_construct_mat3(angle, &mat2);
	
	for(i = 0; i < 8; i++) {
		Vex3D_rot rot;
		rotate_point_local(&rot, &c->v[i], &mat);
		c->v[i].x = rot.x;
		c->v[i].y = rot.y;
		c->v[i].z = rot.z;
	}
	
	for(i = 0; i < 6; i++) {
		Vex3D_rot rot;
		rotate_point_local(&rot, &c->normal[i], &mat);
		c->normal[i].x = rot.x;
		c->normal[i].y = rot.y;
		c->normal[i].z = rot.z;
	}
#endif
	
	for(i = 0; i < 8; i++) {
		c->v[i].x += posx;
		c->v[i].y += posy;
		c->v[i].z += posz;
	}
}

// Takes a 3D polygon and projects all of the points into a 2D polygon
void project_polygon3d(Polygon3D* src, RenderContext* c, Polygon2D* dest) {
	int i;
	
	for(i = 0; i < src->total_v; i++) {
		project_vex3d(c, &src->v[i], &dest->v[i]);
	}
	
	dest->total_v = src->total_v;
}

// Given an array of 8 3D points that define a cube, this selects those
// points that belong to the given face
void cube_get_face(Vex3D v[8], short face,  Vex3D dest[4]) {
	int i;
	
	for(i = 0; i < 4; i++)
		dest[i] = v[cube_vertex_tab[face][i]];
}












