// Header File
// Created 2/16/2015; 7:28:30 PM

#pragma once

#include "3D.h"
#include "sbuffer.h"


extern void asm_gray_tri(register signed short asm("%d0"), register signed short asm("%d1"),
												 register signed short asm("%d2"), register signed short asm("%d3"),
												 register signed short asm("%d4"), register signed short asm("%d5"),
												 register short asm("%d6"));
												 
												 
extern void draw_clip_line(register short asm("%d0"), register short asm("%d1"), register short asm("%d2"), register short asm("%d3"), register void* asm("%a2"));


extern void draw_clip_tri(register signed short asm("%d0"), register signed short asm("%d1"),
												 register signed short asm("%d2"), register signed short asm("%d3"),
												 register signed short asm("%d4"), register signed short asm("%d5"),
												 register void* asm("%a0"));



typedef struct {
	long slope;
	Vex2D start;
	
} Line;

inline void line_info(Line* dest, Vex2D* start, Vex2D* end);
short line_intersect(Line* line, short y);
void rasterize_triangle_full(CBuffer* buf, short x1, short y1, short x2, short y2, short x3, short y3, short color);
char rasterize_triangle_half(CBuffer* buf, long slope_left, long slope_right, short y, short end_y, short* x_left, short* x_right, short color);

#define LINE_BITS 19
#define SWAP(_a,_b) {typeof(_a) _save = _a; _a = _b; _b = _save;}