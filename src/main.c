// C Source File
// Created 3/4/2015; 10:41:26 PM

#include "console.h"
#include "geo.h"

#include <tigcclib.h>

void test_console();

void _main(void) {	
	RenderContext context;
	init_render_context(LCD_WIDTH, LCD_HEIGHT, 0, 0, ANG_90, &context);

	clrscr();
	printf("Scale: %d\n", context.scale);
	ngetchx();
}