// C Source File
// Created 2/16/2015; 7:51:20 PM

#include <tigcclib.h>

void error(const char* msg) {
	GrayOff();
	PortRestore();
	clrscr();
	printf("Error: %s", msg);
	
	while(1) ;
	
	ngetchx();
	
	exit(0);
}