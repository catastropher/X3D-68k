// Header File
// Created 2/21/2015; 1:02:31 AM

#pragma once

typedef struct Seg{
	unsigned char left;
	unsigned char right;
	struct Seg *next;
} Seg;

typedef struct {
	Seg start;
	char full;;
} Scanline;

#define MAX_HEIGHT 240

typedef struct {
	short width;
	short height;
	short scale_factor;
	
	Seg* seg_pool;
	short total_segs;
	short seg_pos;
	
	Scanline lines[MAX_HEIGHT];
	short lines_left;
	
	void* light_plane;
	void* dark_plane;
	
	short draw_mode;
	
	short pix_left_dx;
	short pix_right_dx;
	short pix_left_start;
	short pix_right_start;
	
	unsigned short frame;
	char draw_left, draw_right;
	char outline;
	
	char first_cube;
} CBuffer;

void add_seg(CBuffer* buf, short y, short left, short right, short color);
void init_cbuffer(CBuffer* buf, short segs);
void reset_cbuffer(CBuffer* buf);

#define DRAW_MONO 1
#define DRAW_GRAY 0