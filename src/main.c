// C Source File
// Created 3/4/2015; 10:41:26 PM

#include <tigcclib.h>

#include "console.h"
#include "geo.h"
#include "link.h"
#include "timer.h"

#include "extgraph.h"

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
	init_timers();
}

extern long line_count;

volatile INT_HANDLER old_int_1;
volatile INT_HANDLER old_int_5;

CALLBACK void reset_inthandler() {
	SetIntVec(AUTO_INT_1, old_int_1);
	SetIntVec(AUTO_INT_5, old_int_5);
}

volatile short system_timer;

#define TICKS_PER_SECOND 19

DEFINE_INT_HANDLER(new_auto_int_5) {
	system_timer++;
}

extern short max_recursion_depth;
extern unsigned short clip_count;
extern short plane_clip;
extern char invert_screen;

char cinematic_mode;
char cinematic_size;

char enable_gravity;
char shake;

Vex3D bridge_left_start;
Vex3D bridge_left_end;
Vex3D bridge_right_start;
Vex3D bridge_right_end;

enum {
	CINEMATIC_DISABLED,
	CINEMATIC_ENABLED
};

#ifdef TI89
#define CINEMATIC_MAX 15
#else
#define CINEMATIC_MAX 20
#endif

#define BRIDGE_WIDTH 200

#define SHAKE_MAX 10

Vex3D switch_pos;
char switch_active;

char enable_movement;

short bridge_t;
char extend_bridge;
short bridge_shimmer_t;

Camera save_cam;

void cube_center(short id, Vex3D* center) {
	center->x = 0;
	center->y = 0;
	center->z = 0;
	
	int i;
	for(i = 0; i < 8; i++) {
		add_vex3d(center, &cube_tab[id].v[i], center);
	}
	
	center->x /= 8;
	center->y /= 8;
	center->z /= 8;
}

void stop_shake(void *data) {
	shake = 0;
	
	RenderContext* context = data;
	
	save_cam = context->cam;
	
	context->cam.pos.x = -885;
	context->cam.pos.y = -418;
	context->cam.pos.z = -1310;
	
	set_cam_pos(context, -885, -418, 1310);
	
	set_cam_angle(context, 42, 105, 0);
	enable_movement = 0;
	
	context->cam.current_cube = 38;
	cinematic_mode = 1;
}

void restore_cam(void* data) {
	RenderContext* context = data;
	
	cinematic_mode = 0;
	enable_gravity = 1;
	enable_movement = 1;
	
	context->cam = save_cam;
	
	set_cam_pos(context, context->cam.pos.x, context->cam.pos.y, context->cam.pos.z);
	set_cam_angle(context, context->cam.angle.x, context->cam.angle.y, context->cam.angle.z);
}

void begin_shake(void* data) {
	shake = 1;
	add_timer(50, 0, stop_shake, data);
}


void _main(void) {	
#ifdef TI89
	FontSetSys(F_4x6);
#else
	FontSetSys(F_6x8);
#endif

	clrscr();
	
	int i;
	
#if 1
	cube_tab = malloc(sizeof(Cube) * 30);
	
	//printf("Generating recip tab...\n");
	//gen_recip_tab();
	
	init();
	load_level("level01");
	//create_test_level();
	//ngetchx();
#endif
	
	
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
	
	context.cam.on_ground = 0;
	
	
	context.screen = malloc(LCD_SIZE);
	PortSet(context.screen, 239, 127);
	
	Vex3D center;
	
	short start_cube = 0;
	cube_center(start_cube, &center);
	
	// Calculate the position of the switch for the light bridge
	cube_center(36, &switch_pos);
	switch_pos.x -= 70;
	switch_pos.y += 20;
	switch_pos.z -= 150;
	
	switch_active = 0;
	
	// Calculate the position of the bridge sides
	bridge_left_start.x = cube_tab[36].v[cube_vertex_tab[PLANE_RIGHT][0]].x;
	bridge_left_start.z = (cube_tab[36].v[cube_vertex_tab[PLANE_RIGHT][0]].z + cube_tab[36].v[cube_vertex_tab[PLANE_RIGHT][1]].z) / 2 -
		BRIDGE_WIDTH / 2;
		
	bridge_left_start.y = cube_tab[36].v[cube_vertex_tab[PLANE_RIGHT][0]].y;
	
	bridge_left_end = bridge_left_start;
	bridge_left_end.x = cube_tab[21].v[cube_vertex_tab[PLANE_LEFT][0]].x;
	
	bridge_right_start = bridge_left_start;
	bridge_right_start.z += BRIDGE_WIDTH;
	bridge_right_end = bridge_left_end;
	bridge_right_end.z += BRIDGE_WIDTH;
	
	bridge_t = 0;
	extend_bridge = 0;
	bridge_shimmer_t = -1;
	
	context.cam.velocity.y = 0;
	
	set_cam_pos(&context, center.x, center.y, center.z);
	set_cam_angle(&context, 0, 0, 0);
	
	context.cam.pos_long.x = (long)context.cam.pos.x << NORMAL_BITS;
	context.cam.pos_long.y = (long)context.cam.pos.y << NORMAL_BITS;
	context.cam.pos_long.z = (long)context.cam.pos.z << NORMAL_BITS;
	
	unsigned short key;
	
	old_int_1 = GetIntVec(AUTO_INT_1);
	old_int_5 = GetIntVec(AUTO_INT_5);
	
	SetIntVec(AUTO_INT_1, DUMMY_HANDLER);
	SetIntVec(AUTO_INT_5, new_auto_int_5);
	
	atexit(reset_inthandler);
	
	SetIntVec(INT_VEC_STACK_OVERFLOW, div_by_zero);
	
	context.cam.current_cube = 0;
	
	short frame_count = 0;
	char draw_fps = 0;
	short fps = 0;
	
	short cx = context.center_x;
	short cy = context.center_y;
	
	system_timer = 0;
	
	// Reset cinematic mode
	cinematic_mode = 0;
	cinematic_size = 0;
	
	enable_gravity = 1;
	enable_movement = 1;
	
	shake = 0;
	
	char added_restore_timer = 0;
	
	do {
		key = read_keys();
		
		clrscr();
		
		if(extend_bridge && context.cam.pos.x <= bridge_left_start.x + 15 && 
			context.cam.pos.x >= bridge_left_end.x - 15 &&
			context.cam.pos.z >= bridge_left_start.z && 
			context.cam.pos.z <= bridge_right_start.z) {
			//context.cam.pos.y - PLAYER_HEIGHT + 10 < bridge_left_start.y) {
				
				enable_gravity = 0;
		} else {
			enable_gravity = 1;
		}
		
		line_count = 0;
		clip_count = 0;
		plane_clip = 0;
		invert_screen = 0;
		
		process_timers();
		
		if(extend_bridge) {
			if(bridge_t < 256) {
				bridge_t += 8;
			}
			else {
				if(!added_restore_timer) {
					add_timer(20, 0, restore_cam, &context);
					added_restore_timer = 1;
					bridge_shimmer_t = 0;
				}
			}
		}
		
		if(shake) {
			context.center_x = cx + (rand() % (2 * SHAKE_MAX)) - SHAKE_MAX;
			context.center_y = cy + (rand() % (2 * SHAKE_MAX)) - SHAKE_MAX;
		}
		
		if(cinematic_mode == CINEMATIC_ENABLED) {
			if(cinematic_size < CINEMATIC_MAX)
				cinematic_size += 4;
			else
				extend_bridge = 1;
		}
		
		if(cinematic_mode && cinematic_size > 0) {
			FastFilledRect_Draw_R(context.screen, 0, 0, LCD_WIDTH - 1, cinematic_size - 1);
			FastFilledRect_Draw_R(context.screen, 0, LCD_HEIGHT - cinematic_size, LCD_WIDTH - 1, LCD_HEIGHT - 1);
		}
		
		render_level(&context);
		
		if(bridge_shimmer_t >= 0) {
			bridge_shimmer_t += 8;
			
			if(bridge_shimmer_t >= 256)
				bridge_shimmer_t = 0;
		}
		
	#if 0
		if(invert_screen) {
			for(i = 0; i < LCD_SIZE; i++)
				context.screen[i] = ~context.screen[i];
		}
	#endif
		
		frame_count++;
		
		//printf("Line count: %ld\n", line_count);
		
		if(draw_fps) {
			printf("fps: %d\nClip count: %u\nCube count: %ld\nPlane clip: %d\n", fps, clip_count, line_count, plane_clip);//\ndepth: %d\n", fps, max_recursion_depth);
			//printf("DDDD: %d\n", context.frustum.p[0].d);
		}
		
		//print_vex3d(&context.cam.dir);
		
		if(system_timer >= TICKS_PER_SECOND) {
			draw_fps = 1;
			fps = frame_count;
			frame_count = 0;
			system_timer = 0;
		}
		
		if(!switch_active && dist(&context.cam.pos, &switch_pos) < 100) {
			
#ifdef TI89
			FastFilledRect_Draw_R(context.screen, 0, 0, LCD_WIDTH - 1, 7);
#else
			FastFilledRect_Draw_R(context.screen, 0, 0, LCD_WIDTH - 1, 10);
#endif

			DrawStr(0, 1, "Press APPS to enable light bridge", A_REVERSE);
		}
		
		//print_plane(&context.frustum.p[0]);
		
		
		context.frame++;
		
		
		// Reset X and Z velocity, which will be set based upon which keys are held down
		context.cam.velocity.x = 0;
		context.cam.velocity.z = 0;
		
		if(enable_gravity)
			context.cam.velocity.y = 32768L * 30;
		else
			context.cam.velocity.y = 0;
		
		const long MOVE_SPEED = 10;
		const long FAST_SPEED = MOVE_SPEED * 3;
		
		if(_keytest(RR_F1)) {
			context.cam.velocity.x = MOVE_SPEED * context.cam.dir.x;
			context.cam.velocity.z = MOVE_SPEED * context.cam.dir.z;
		}
		else if(_keytest(RR_F2)) {
			context.cam.velocity.x = -MOVE_SPEED * context.cam.dir.x;
			context.cam.velocity.z = -MOVE_SPEED * context.cam.dir.z;
		}
		else if(_keytest(RR_F5)) {
			context.cam.velocity.x = FAST_SPEED * context.cam.dir.x;
			context.cam.velocity.z = FAST_SPEED * context.cam.dir.z;
		}
		
		if(key & GAME_KEY_RIGHT) {
			set_cam_angle(&context, context.cam.angle.x, context.cam.angle.y - 3, context.cam.angle.z);
		}
		
		if(key & GAME_KEY_LEFT) {
			set_cam_angle(&context, context.cam.angle.x, context.cam.angle.y + 3, context.cam.angle.z);
		}
		
		if(key & GAME_KEY_UP) {
			set_cam_angle(&context, context.cam.angle.x - 3, context.cam.angle.y, context.cam.angle.z);
		}
		
		if(key & GAME_KEY_DOWN) {
			set_cam_angle(&context, context.cam.angle.x + 3, context.cam.angle.y, context.cam.angle.z);
		}
		
		if(_keytest(RR_ENTER)) {
			print_vex3d(&context.cam.pos);
			printf("{%d, %d, %d}\n", context.cam.angle.x, context.cam.angle.y, context.cam.angle.z);
			printf("Cube: %d\n", context.cam.current_cube);
		}
		
		if(_keytest(RR_APPS)) {
			while(_keytest(RR_HAND)) ;
			
			if(dist(&context.cam.pos, &switch_pos) < 100) {
				if(!switch_active) {
					switch_active = 1;
					add_timer(20, 0, begin_shake, &context);
				}
			}
		}
		
	#if 0
		if(_keytest(RR_F6)) {
			while(_keytest(RR_F6)) ;
			
			enable_gravity = !enable_gravity;
		}
		
		if(_keytest(RR_F7)) {
			while(_keytest(RR_F7)) ;
			
			shake = !shake;
			
			if(!shake) {
				context.center_x = cx;
				context.center_y = cy;
			}
			else {
				add_timer(60, 0, stop_shake, NULL);
			}
		}
		
		if(_keytest(RR_APPS)) {
			cinematic_mode = CINEMATIC_ENABLED;
		}
	#endif
		
		char plane;
		
		printf("In cube: %d\n", point_in_cube(context.cam.current_cube, &context.cam.pos, &plane));
		
		
	#if 0
		if(_keytest(RR_F3)) {
			attempt_move_cam(&context, &context.cam.straif_dir, -10, MOVE_AXIS_NOT_Y);
		}
		
		if(_keytest(RR_F4)) {
			attempt_move_cam(&context, &context.cam.straif_dir, 10, MOVE_AXIS_NOT_Y);
		}
		
		if(_keytest(RR_F5)) {
			attempt_move_cam(&context, &context.cam.dir, 60, MOVE_AXIS_NOT_Y);
		}
		
		if(_keytest(RR_F6)) {			
			set_cam_angle(&context, 0, ((context.cam.angle.y + 64) / 64) * 64, 0);
			
			while(_keytest(RR_F6)) ;
		}
	#endif
		
		if(enable_movement)
			attempt_move_cam(&context, &context.cam.velocity, 1, MOVE_AXIS_ALL);
		//attempt_move_cam(&context, &(Vex3D){0, 32767, 0}, 10, MOVE_AXIS_ALL);
	
		short i;
		
		//for(i = 0; i < LCD_SIZE; i++)
		//	context.screen[i] = ~context.screen[i];
	
		LCD_restore(context.screen);
	} while(!(key & GAME_KEY_ESC));
		
		
	PortRestore();
		
	free(context.screen);
	
	
	SetIntVec(AUTO_INT_1, old_int_1);
	SetIntVec(AUTO_INT_5, old_int_5);
	
	free(cube_tab);
	free(recip_tab);
	
}