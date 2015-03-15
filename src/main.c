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
	GAME_KEY_ESC = 64,
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

char test_link() {
	const char* send_str[] = {
		"You are talking with calc 0",
		"You are talking with calc 1"
	};
	
	char str[257];
	
	if(!link_handle_cmd(LINK_STRING, (char *)send_str[(short)calc_id], str))
		return 0;
		
	printf("Calc says: %s\n", str);
	//ngetchx();
	
	return 1;
}

DEFINE_INT_HANDLER(div_by_zero) {
	PortRestore();
	printf("Error: div by 0");
	while(1) ;
}

void init() {
	init_render();
}

extern long line_count;

volatile INT_HANDLER old_int_1;
volatile INT_HANDLER old_int_5;

CALLBACK void reset_inthandler() {
	SetIntVec(AUTO_INT_1, old_int_1);
	SetIntVec(AUTO_INT_5, old_int_5);
}



void _main(void) {	
	FontSetSys(F_6x8);
	clrscr();
	
	
#if 0
	cleanup_link();
	
	char connected = link_connect();
	
	if(!connected) {
		cleanup_link();
		return;
	}
		
	if(!test_link()) {
		printf("Error\n");
		cleanup_link();
		while(ngetchx() != 'q');
		return;
	}
	
	//cleanup_link();
	//while(ngetchx() != 'q');
	
	
	
	//return;
	
	show_console_chat();
	cleanup_link();
	
	while(ngetchx() != 'q') ;
	
	return;
#endif
	
	// Create and initialize the rendering context
	RenderContext context;
	init_render_context(LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_90, &context);
	
	context.screen = LCD_MEM;
	//test_polygon_clipper(&context);
	
	init();
	create_test_level();
	
	context.screen = malloc(LCD_SIZE);
	PortSet(context.screen, 239, 127);
	
	// Initialize the camera
	//set_cam_pos(&context, -25, 12, -85);
	//set_cam_angle(&context, 0, 0, 0);
	
	//set_cam_pos(&context, -197, -98, 326);
	//set_cam_angle(&context, 26, 192, 0);
	
	set_cam_pos(&context, -146, -80, 398);
	set_cam_angle(&context, 252, 138, 0);
	
	//print_frustum(&context.frustum);
	
	// Create a test cube
	Cube cube, cube2;
	Vex3Ds cube_angle = {0, 0, 0};
	unsigned short key;
	
	old_int_1 = GetIntVec(AUTO_INT_1);
	old_int_5 = GetIntVec(AUTO_INT_5);
	
	SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
	SetIntVec(AUTO_INT_5, DUMMY_HANDLER);
	
	atexit(reset_inthandler);
	
	//SetIntVec(INT_VEC_STACK_OVERFLOW, div_by_zero);
	
	context.cam.current_cube = 1;
	
	do {
		key = read_keys();
		
		clrscr();
		
		line_count = 0;
		render_level(&context);
		
		printf("Line count: %ld\n", line_count);
		
		//print_plane(&context.frustum.p[0]);
		
		
		context.frame++;
		
		if(key & GAME_KEY_F1) {
			//set_cam_pos(&context, context.cam.pos.x + context.cam.dir.x / 4096,
			//	context.cam.pos.y + context.cam.dir.y / 4096,
			//	context.cam.pos.z + context.cam.dir.z / 4096);
			
			attempt_move_cam(&context, &context.cam.dir, 10);
			
		}
		
		if(key & GAME_KEY_F2) {
			//set_cam_pos(&context, context.cam.pos.x - context.cam.dir.x / 4096,
			//	context.cam.pos.y - context.cam.dir.y / 4096,
			//	context.cam.pos.z - context.cam.dir.z / 4096);
			attempt_move_cam(&context, &context.cam.dir, -10);
		}
		
		
		if(_keytest(RR_F3)) {
			attempt_move_cam(&context, &context.cam.straif_dir, -10);
		}
		
		if(_keytest(RR_F4)) {
			attempt_move_cam(&context, &context.cam.straif_dir, 10);
		}
		
		
		if(key & GAME_KEY_RIGHT) {
			set_cam_angle(&context, context.cam.angle.x, context.cam.angle.y - 2, context.cam.angle.z);
		}
		
		if(key & GAME_KEY_LEFT) {
			set_cam_angle(&context, context.cam.angle.x, context.cam.angle.y + 2, context.cam.angle.z);
		}
		
		if(key & GAME_KEY_UP) {
			set_cam_angle(&context, context.cam.angle.x - 2, context.cam.angle.y, context.cam.angle.z);
		}
		
		if(key & GAME_KEY_DOWN) {
			set_cam_angle(&context, context.cam.angle.x + 2, context.cam.angle.y, context.cam.angle.z);
		}
		
		if(_keytest(RR_ENTER)) {
			print_vex3d(&context.cam.pos);
			printf("{%d, %d, %d}\n", context.cam.angle.x, context.cam.angle.y, context.cam.angle.z);
			printf("Cube: %d\n", context.cam.current_cube);
		}
	
		short i;
		
		//for(i = 0; i < LCD_SIZE; i++)
		//	context.screen[i] = ~context.screen[i];
	
		LCD_restore(context.screen);
	} while(!(key & GAME_KEY_ESC));
		
		
	PortRestore();
		
	free(context.screen);
	
	
	SetIntVec(AUTO_INT_1, old_int_1);
	SetIntVec(AUTO_INT_5, old_int_5);
	
}