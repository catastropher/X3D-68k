// C Source File
// Created 3/4/2015; 10:41:26 PM

#include "console.h"
#include "geo.h"

#include <tigcclib.h>

void test_console();

void _main(void) {	
	FontSetSys(F_6x8);
	clrscr();
	
	// Create and initialize the rendering context
	RenderContext context;
	init_render_context(LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_90, &context);
	
	// Initialize the camera
	set_cam_pos(&context, 0, 0, 0);
	set_cam_angle(&context, 0, 0, 0);
	
	// Create a test cube
	Cube cube;
	Vex3Ds cube_angle = {0, 0, 0};
	construct_cube(100, 100, 100, 0, 0, 200, &cube_angle, &cube);
	
	render_cube(&cube, &context);
	
	
	
	ngetchx();
}