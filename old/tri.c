// C Source File
// Created 2/20/2015; 9:10:08 PM

#include <tigcclib.h>

#include "graphics.h"
#include "sbuffer.h"

extern short lcd_w, lcd_h;


void render_seg_callback(short left asm("d0"), short right asm("d1"), void* scanline asm("a0")) {
	
}

#define SWAP(_a,_b) {typeof(_a) _save = _a; _a = _b; _b = _save;}

char rasterize_triangle_half(CBuffer* buf, long slope_left, long slope_right, short y, short end_y, short* x_left, short* x_right, short color) {
	char ret = 0;
	short save_end = end_y;
	
	if(end_y >= buf->height) {
		end_y = buf->height - 1;
		ret = 1;
	}
	
	if((*x_left == *x_right && slope_left > slope_right) || *x_left > *x_right) {
		SWAP(slope_left, slope_right);
		SWAP(x_left, x_right);
	}
	
	long left = (long)*x_left << LINE_BITS;
	long right = (long)*x_right << LINE_BITS;
	
	short last_l = *x_left;
	short last_r = *x_right;
	short l, r;
	
	short prev_left;
	
	while(y <= end_y && buf->lines_left > 0) {
		l = left >> LINE_BITS;
		r = right >> LINE_BITS;
		
		buf->pix_left_start = l;
		buf->pix_right_start = r;
		buf->pix_left_dx = l - last_l;
		buf->pix_right_dx = r - last_r;
		
		if(!buf->lines[y].full)
			add_seg(buf, y, l, r, color);
			
		left += slope_left;
		right += slope_right;
		++y;
		//printf("y = %d\n", y);
		
		last_l = l;
		last_r = r;
	}
	
	*x_left = (left - slope_left) >> LINE_BITS;
	*x_right = (right - slope_right) >> LINE_BITS;
	
	return y == save_end || ret || buf->lines_left == 0;
}

void rasterize_triangle_full(CBuffer* buf, short x1, short y1, short x2, short y2, short x3, short y3, short color) {
	Vex2D v2[3] = {
		{x1, y1},
		{x2, y2},
		{x3, y3}
	};
	
	char changed;
	int i;

	
	do {
		changed = 0;
		for(i = 0; i < 2; i++)
			if(v2[i].y > v2[i + 1].y) {
				changed = 1;
				Vex2D temp = v2[i];
				v2[i] = v2[i + 1];
				v2[i + 1] = temp;
			}
	} while(changed);
	
	Line a, b, c;
	
	if(v2[0].y == v2[2].y)		// Degenerate triangle
		return;
	if(v2[0].y == v2[1].y) {
		line_info(&a, &v2[0], &v2[2]);
		line_info(&b, &v2[1], &v2[2]);
		
		rasterize_triangle_half(buf, a.slope, b.slope, v2[0].y, v2[2].y, &v2[0].x, &v2[1].x, color);
	}
	else if(v2[1].y == v2[2].y) {
		line_info(&a, &v2[0], &v2[1]);
		line_info(&b, &v2[0], &v2[2]);
		
		rasterize_triangle_half(buf, a.slope, b.slope, v2[0].y, v2[1].y, &v2[0].x, &v2[0].x, color);
	}
	else {
		line_info(&a, &v2[0], &v2[1]);
		line_info(&b, &v2[0], &v2[2]);
		short temp_x = v2[0].x;
		
		if(!rasterize_triangle_half(buf, a.slope, b.slope, v2[0].y, v2[1].y, &temp_x, &v2[0].x, color)) {
			line_info(&c, &v2[1], &v2[2]);
			rasterize_triangle_half(buf, b.slope, c.slope, v2[1].y, v2[2].y, &v2[0].x, &v2[1].x, color);
		}
	}
}




