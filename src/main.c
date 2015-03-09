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
	
	int i;
	for(i = 0; i < context.frustum.total_p; i++) {
		print_plane(&context.frustum_unrotated.p[i]);
		ngetchx();
	}
	
	printf("Scale: %d\n", context.dist);
	ngetchx();
	
	context.screen = LCD_MEM;//malloc(LCD_SIZE);
	//PortSet(context.screen, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	
	// Initialize the camera
	set_cam_pos(&context, 0, 0, 0);
	set_cam_angle(&context, 0, 0, 0);
	
	// Create a test cube
	Cube cube, cube2;
	Vex3Ds cube_angle = {0, 0, 0};
	construct_cube(100, 100, 100, 0, 0, 1000, &cube_angle, &cube);
	construct_cube(100, 100, 100, 0, 0, 400, &cube_angle, &cube2);
	
	clrscr();
	render_cube(&cube, &context);
	//render_cube(&cube2, &context);
	
	//LCD_restore(context.screen);
	
	ngetchx();
	
	//PortRestore();
	//free(context.screen);
}