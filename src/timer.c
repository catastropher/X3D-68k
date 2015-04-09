// C Source File
// Created 4/8/2015; 10:47:03 AM

#include <tigcclib.h>

#include "timer.h"
#include "error.h"

short total_timers;

Timer timer_tab[MAX_TIMERS];

// Initializes the timers
void init_timers() {
	short i;
	
	for(i = 0; i < MAX_TIMERS; i++)
		timer_tab[i].ticks = 0;
		
	total_timers = 0;
}

void add_timer(short ticks, short repeat, void (*callback)(void* data), void* data) {
	short i;
	
	for(i = 0; i < MAX_TIMERS; i++) {
		if(timer_tab[i].ticks == 0) {
			timer_tab[i].ticks = ticks;
			timer_tab[i].start_ticks = ticks;
			timer_tab[i].repeat = repeat;
			timer_tab[i].callback = callback;
			timer_tab[i].data = data;
			return;
		}
	}
	
	error("Out of timers");
}

void process_timers() {
	short i;
	
	for(i = 0; i < MAX_TIMERS; i++) {
		if(timer_tab[i].ticks != 0) {
			if(--timer_tab[i].ticks == 0) {
				timer_tab[i].callback(timer_tab[i].data);
				
				if(timer_tab[i].repeat > 0) {
					--timer_tab[i].repeat;
					timer_tab[i].ticks = timer_tab[i].start_ticks;
				}
			}
		}
	}
}