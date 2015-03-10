// C Source File
// Created 3/9/2015; 11:23:36 AM

#include "link.h"

#include <tigcclib.h>

// Throws an error, prints out the message, and then quits the program
void error(const char* format, ...) {
	char buf[512];
	va_list list;
	
	cleanup_link();
	
	va_start(list, format);
	vsprintf(buf, format, list);
	
	PortRestore();
	clrscr();
	printf("Error: %s\n", buf);
	ngetchx();
	exit(-1);
}