// C Source File
// Created 2/16/2015; 7:28:15 PM

#include "mem.h"
#include "error.h"
#include "extgraph.h"
#include "graphics.h"
#include "mem.h"
#include "3D.h"
#include "sbuffer.h"

#include <tigcclib.h>

unsigned short lcd_w, lcd_h;
void *Vscreen0, *Vscreen1;

void cleanup(void) {
	GrayOff();
	PortRestore();
}

void init() {
	init_mem();
	
	#ifdef MODE_GRAY
	if(!GrayOn())
		error("Failed to init grayscale");
		
	void* gray_dbuf = alloc_mem(GRAYDBUFFER_SIZE);
	GrayDBufInit(gray_dbuf);
	
	#endif
	
	atexit(cleanup);
	
}

#define DEG_TO_RAD(deg) (deg*(PI/180.0))

short *sin_tab;

char x3d_init(){
	int i;
	
	clrscr();
	
	printf("Alloc sin_tab\n");
	//allocate the sine tab
	sin_tab = alloc_mem(360*sizeof(short));
	
	printf("Alloc recip tab\n");
	//recip_tab = malloc(1024*sizeof(short));
	
	//goto skip;
	
	printf("Searching for X3Dtab\n");
	//load the sintab from a file, if it exists
	FILE *f = fopen("X3Dtab","rb");
	
	if(f == 0){	//the file hasn't been created yet
		printf("Not found\nGenerating sin tab\n");
		
		//generate the sin tab
		for(i = 0;i<360;i++){
			sin_tab[i] = sin(DEG_TO_RAD(i))*32768.0;
		}
		
		sin_tab[90] = 32767;
		sin_tab[270] = -32767;
		
		printf("Writing to file\n");
		f = fopen("X3Dtab","wb");
		
		fwrite(sin_tab,2,360,f);
	}
	else{	//the file's already been created
		printf("Reading file\n");
		fread(sin_tab,2,360,f);
	}
	
	fclose(f);
	
skip:
	
	//set up key repeat
	//old_init_key = OSInitKeyInitDelay(1);
	//old_between_key = OSInitBetweenKeyDelay(5);
	
	
	printf("X3D init success\n");
	
	
//	PortSet(screen,239,127);

	printf("X3D init\nEnable grayscale\n");
	return 1;
		
}

void test_cube(CBuffer* buf, Cam* cam, char outline, Vex3D *cube_angle);
void test_vector();
void test_rotate();

void cam_update_dir(Cam* cam) {
	cam->dir.x = cam->mat[2][0] >> (SIN_BITS - NORMAL_BITS);
	cam->dir.y = cam->mat[2][1] >> (SIN_BITS - NORMAL_BITS);
	cam->dir.z = cam->mat[2][2] >> (SIN_BITS - NORMAL_BITS);
}

void adjust_gray() {
	int value = 0;
	
	do {
		Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
		Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
		ClearGrayScreen2B(Vscreen0, Vscreen1);
		
		memset(Vscreen0, 0xFF, LCD_SIZE / 2);
		memset(Vscreen1 + LCD_SIZE / 2, 0xFF, LCD_SIZE / 2);
		
		char buf[32];
		
		sprintf(buf, "Value: %d", value);
		
	
		PortSet(Vscreen1, 239, 127);
		DrawStr(0, 0, buf, A_NORMAL);
		GrayDBufToggleSync();
		
		unsigned short key = ngetchx();
		
		if(key == KEY_ESC)
			return;
		else if(key == KEY_UP)
			OSContrastUp();
		else if(key == KEY_DOWN)
			OSContrastDn();
		else if(key == '+')
			value++;
		else if(key == '-')
			if(value > 0)
				value--;
		
		GrayAdjust(value);
	} while(1);
}

void print_segs();

void test_cbuffer() {
	#if 0
	init_cbuffer(2000);
	
	Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
	Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
	ClearGrayScreen2B(Vscreen0, Vscreen1);
	
	reset_cbuffer();
	rasterize_triangle_full(20, 20, 20, 50, 100, 100, COLOR_DARKGRAY);
	rasterize_triangle_full(10, 10, 10, 110, 120, 110, COLOR_LIGHTGRAY);
	rasterize_triangle_full(130, 10, 130, 100, 170, 10, COLOR_LIGHTGRAY);
	
	rasterize_triangle_full(10, 10, 20, 110, 200, 110, COLOR_BLACK);
	GrayDBufToggleSync();
	
	print_segs();
	
	clrscr();
	render_segs();
	
	ngetchx();
	
	#endif
	
}

void init_cubes();
void invert_normals();
void build_edge_table();

extern short scale_factor;

extern long seg_calls;

void test_polygon_clipper();

// Main Function
void _main(void) {
	//DrawGrayRect(50, 50, 100, 100, COLOR_DARKGRAY, RECT_FILLED);

	test_polygon_clipper();
	return 0;
	
	printf("Hello");
	ngetchx();
	
	unsigned short key;
	unsigned short color = 1;
	int angle = 0;
	int angle2 = 0;
	char outline = 0;
	int fps = 0;
	int frames = 0;
	int count = 0;
	int speed = 1;
	long calls = 0;
	
	CBuffer buf;
	
	buf.width = 240;
	buf.height = 128;
	buf.scale_factor = 208;
	buf.seg_pos = 0;
	buf.frame = 0;
	buf.dark_plane = alloc_mem(LCD_SIZE);
	buf.lines_left = 1000;
	
	Cam cam;
	
	cam.pos.x = 0;
	cam.pos.y = 0;
	cam.pos.z = -100;
	
	cam.angle.x = 0;
	cam.angle.y = 0;
	cam.angle.z = 0;
	
	cam.dir.z = 4096;
	cam.dir.x = 0;
	cam.dir.y = 0;
	
	Vex3D cube_angle = {0, 0, 0};
	
	void* kbd = kbd_queue();
	int timer;
	
	OSFreeTimer(1);
	for(timer = 1; timer <= 6; timer++)
		if(OSRegisterTimer(timer, 20))
			break;
	
	clrscr();
	printf("X3D build 3\n");
	printf("Author: Michael Wilder\n");
	//ngetchx();
			
	//printf("Timer: %d", timer);
	//ngetchx();
	
	x3d_init();
	//init_cbuffer(&buf, 2000);
	init_cubes();
	build_edge_table();
	//LCD_restore(buf.dark_plane);
	ngetchx();
	//test_vector();
	//test_rotate();
	
	printf("Please adjust grayscale\nPress [enter]");
	init();
	//adjust_gray();
	
	int old_init_key = OSInitKeyInitDelay(1);
	int old_between_key = OSInitBetweenKeyDelay(5);
	
	//test_cbuffer();
	
	x3d_construct_mat3(&cam.angle, &cam.mat);
	cam_update_dir(&cam);
			
	if(timer == 7)
		error("No free timers");
	
	lcd_w = 240;
	lcd_h = 128;
	scale_factor = 208;
	
	PortSet(buf.dark_plane, 239, 127);
	
	do {
		char fps_str[64];
		
	#ifdef MODE_GRAY
		Vscreen0 = GrayDBufGetHiddenPlane(LIGHT_PLANE);
		Vscreen1 = GrayDBufGetHiddenPlane(DARK_PLANE);
		ClearGrayScreen2B(Vscreen0, Vscreen1);
		
		buf.light_plane = GrayDBufGetHiddenPlane(LIGHT_PLANE);
		buf.dark_plane = GrayDBufGetHiddenPlane(DARK_PLANE);
	#else
		clrscr();
	#endif
		
		if(OSTimerExpired(timer)) {
			fps = frames;
			OSTimerRestart(timer);
			frames = 0;
			calls = seg_calls;
			seg_calls = 0;
		}
		
		//asm_gray_tri(10, 10, 200, 50, 100, 100, color);
		//GrayDBufToggleSync();
	#ifdef MODE_GRAY
		reset_cbuffer(&buf);
	#endif
		//rasterize_triangle_full(&buf, 20, 20, 20, 50, 100, 100, COLOR_DARKGRAY);
		test_cube(&buf, &cam, outline, &cube_angle);
		outline = 0;
		buf.frame++;
		
	#ifdef MODE_GRAY
		GrayDBufToggleSync();
	#else
	#endif
		
		frames++;
		//key = ngetchx();
		
		color = (color + 1) % 4;
		cube_angle.y = (cube_angle.y + speed) % 360;
		//cam.angle.x = cam.angle.y;
		
		if(fps != 0) {
		#if 1
			sprintf(fps_str, "fps: %d, calls: %ld", fps, calls);
			//sprintf(fps_str, "{%d, %d, %d}", cam.dir.x, cam.dir.y, cam.dir.z);
			//PortSet(Vscreen0, 239, 127);
			DrawStr(0, 0, fps_str, A_NORMAL);
			//PortSet(Vscreen1, 239, 127);
			//DrawStr(0, 0, fps_str, A_NORMAL);
		#endif
		}
		
		LCD_restore(buf.dark_plane);
		
		//count++;
		if(count == 4) {
			cam.angle.z = (angle2 + speed) % 360;
			count = 0;
		}
		
		if(OSdequeue(&key, kbd))
			key = 0;
			
		key &= 0xF7FF;
		
		if(key == KEY_ENTER)
			outline = !outline;
		
		if(key == KEY_RIGHT) {
			if(speed > 0) {
			//	speed--;
			}
			cam.angle.y -= 3;
			
			if(cam.angle.y < 0)
				cam.angle.y += 360;
			
			x3d_construct_mat3(&cam.angle, &cam.mat);
			cam_update_dir(&cam);
		}
		else if(key == KEY_LEFT) {
			//speed++;
			cam.angle.y += 3;
			
			if(cam.angle.y >= 360)
				cam.angle.y -= 360;
			
			x3d_construct_mat3(&cam.angle, &cam.mat);
			cam_update_dir(&cam);
		}
		else if(key == KEY_DOWN) {
			cam.angle.x += 3;
						
			if(cam.angle.x >= 360)
				cam.angle.x -=360;
			
			x3d_construct_mat3(&cam.angle, &cam.mat);
			cam_update_dir(&cam);
		}
		else if(key == KEY_UP) {
			cam.angle.x -= 3;
			
			if(cam.angle.x < 0)
				cam.angle.x += 360;
				
			x3d_construct_mat3(&cam.angle, &cam.mat);
			cam_update_dir(&cam);
		}
		else if(_keytest(6, 4)) {
			cam.pos.x += cam.dir.x / 8192;
			cam.pos.y += cam.dir.y / 8192;
			cam.pos.z += cam.dir.z / 8192;
			//cam.pos.y -= 8;
		}
		else if(_keytest(4, 4)) {
			cam.pos.x -= cam.dir.x / 8192;
			cam.pos.y -= cam.dir.y / 8192;
			cam.pos.z -= cam.dir.z / 8192;
			//cam.pos.y += 8;
		}
		else if(key == KEY_F3) {
			speed++;
		}
		else if(key == KEY_F4) {
			if(speed > 0)
				speed--;
		}
		else if(key == KEY_F5) {
			invert_normals();
		}
		else if(key == KEY_F6) {
			if(buf.width == 240) {
				buf.width = 240 / 2;
				buf.height = 128 / 2;
				buf.scale_factor = 208 / 2;
			}
			else {
				buf.width = 240;
				buf.height = 128;
				buf.scale_factor = 208;
			}
		}
					
	} while(key != KEY_ESC);
	
#ifdef MODE_GRAY
	GrayOff();
#endif
	
	OSInitKeyInitDelay(old_init_key);
	OSInitBetweenKeyDelay(old_between_key);
	
	OSFreeTimer(timer);
		
}