// Header File
// Created 4/8/2015; 10:47:10 AM
#pragma once


#define MAX_TIMERS 10

typedef struct {
	short ticks;
	short start_ticks;
	short repeat;
	void* data;
	void (*callback)(void* data);
} Timer;

void add_timer(short ticks, short repeat, void (*callback)(void* data), void* data);
void init_timers();
void process_timers();