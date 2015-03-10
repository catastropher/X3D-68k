// C Source File
// Created 3/4/2015; 10:41:26 PM

#include "console.h"
#include "geo.h"
#include "link.h"

#include <tigcclib.h>

void test_console();

enum {
	GAME_KEY_UP = 1,
	GAME_KEY_DOWN = 2,
	GAME_KEY_LEFT = 4,
	GAME_KEY_RIGHT = 8,
	GAME_KEY_F1 = 16,
	GAME_KEY_F2 = 32,
	GAME_KEY_ESC = 64
};

unsigned short read_keys() {
	unsigned short res = _keytest(RR_UP) |
		((unsigned short)_keytest(RR_DOWN) << 1) |
		((unsigned short)_keytest(RR_LEFT) << 2) |
		((unsigned short)_keytest(RR_RIGHT) << 3) |
		((unsigned short)_keytest(RR_F1) << 4) |
		((unsigned short)_keytest(RR_F2) << 5) |
		((unsigned short)_keytest(RR_ESC) << 6);
		
	return res;
		
}

void _main(void) {	
	FontSetSys(F_6x8);
	clrscr();
	
	char connected = link_connect();
	
	printf("Connected: %d\n", connected);
	ngetchx();
	cleanup_link();
	
	return;
	
	
	// Create and initialize the rendering context
	RenderContext context;
	init_render_context(LCD_WIDTH, LCD_HEIGHT, 0, 0, 42, &context);
	
	int i;
	for(i = 0; i < context.frustum.total_p; i++) {
		//print_plane(&context.frustum_unrotated.p[i]);
		//ngetchx();
	}
	
	printf("Scale: %d\n", context.dist);
	ngetchx();
	
	context.screen = malloc(LCD_SIZE);
	PortSet(context.screen, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	
	// Initialize the camera
	set_cam_pos(&context, 0, 0, 0);
	set_cam_angle(&context, 0, 0, 0);
	
	// Create a test cube
	Cube cube, cube2;
	Vex3Ds cube_angle = {0, 0, 0};
	construct_cube(100, 100, 100, 0, 0, 400, &cube_angle, &cube);
	construct_cube(100, 100, 100, 0, 0, 400, &cube_angle, &cube2);
	
	unsigned short key;
	
	do {
		key = read_keys();
		clrscr();
		render_cube(&cube, &context);
		//render_cube(&cube2, &context);
		
		if(key & GAME_KEY_F1) {
			set_cam_pos(&context, context.cam.pos.x, context.cam.pos.y, context.cam.pos.z + 1);
		}
	
		LCD_restore(context.screen);
	} while(!(key & GAME_KEY_ESC));
		
	PortRestore();
		
	free(context.screen);
}