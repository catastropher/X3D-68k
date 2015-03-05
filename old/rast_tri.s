| Assembly Source File
| Created 2/22/2015; 11:01:24 AM


|char rasterize_triangle_half(CBuffer* buf, long slope_left, long slope_right, short y, short end_y, short* x_left, short* x_right, short color) {

| a0 -> CBuffer buf;
| d1 -> long slope_left
| d2 -> long slope_right
| d3 -> short y;
| d4 -> short end_y;
| a1 -> short* x_left;
| a2 -> short* x_right;
| d5 -> short color

|cmp d0, d1 -> compare d1 to d0

rast_triangle_half:
	| d6 -> last_l
	| d7 -> last_r
	
	| if the bottom of the triangle is off the screen, clip it
	cmp.w 4(%a0),%d4
	blt no_clip_bottom
	move.w 4(%a0),%d4
	
no_clip_bottom:
	| calculate the height of the triangle piece (number of times to run the loop!)
	sub.w %d3,%d4
	
	| load in initial values for last_l and last_r