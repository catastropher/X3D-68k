// C Source File
// Created 3/30/2015; 10:27:05 AM

#include <tigcclib.h>

#include "geo.h"
#include "error.h"

short* recip_tab;

// Fast reciprocal using a table lookup
inline short fast_recip(short den) {
	return (den < 0 ? -recip_tab[(-den) >> 1] : recip_tab[den >> 1]) << 1;
}

// Fast fixed point arithmetic division
inline short fast_div_fix(short n, short d, short shift) {
	return ((long)n * fast_recip(d)) >> shift;
}

// Generates the reciprocal table
void gen_recip_tab() {
	unsigned int i;
	
	recip_tab = malloc(sizeof(short) * 16384L);
	
	xassert(recip_tab);
	
	for(i = 1; i < 16384; i++) {
		recip_tab[i] = (2L << NORMAL_BITS) / i;
	}
	
	recip_tab[0] = 0x7FFF;
}