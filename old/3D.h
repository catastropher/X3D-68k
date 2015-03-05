// Header File
// Created 2/17/2015; 12:06:47 AM

#pragma once

typedef short Mat3x3[3][3];

// These don't belong here!
short mul_fps(short asm("d0"),short asm("d1"));
short mul_fps_frac(short asm("d0"),short asm("d1"));

typedef struct {
	short x, y, z;
} Vex3D;

void x3d_construct_mat3(Vex3D *angle, Mat3x3 *dest);
void x3d_construct_mat3_old(Vex3D *angle, Mat3x3 *dest);

void mul_mat3x3(Mat3x3* a, Mat3x3* b, Mat3x3* res);

typedef struct {
	Vex3D pos;
	Mat3x3 mat;
	Vex3D dir;
	
	Vex3D angle;
} Cam;

typedef struct {
	short x, y;
} Vex2D;


#define NORMAL_BITS 15
#define NORMAL_MAX_VALUE (((long)1 << NORMAL_BITS) - 1)
#define SIN_BITS 15