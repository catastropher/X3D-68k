// Header File
// Created 3/5/2015; 9:03:11 AM
#pragma once

#define FIXDIV8(_n, _d) (((long)(_n) << 8) / (_d))

#define FIXMULN(_a, _b, _n) (((long)(_a) * (_b)) >> (_n))




#define FIXMUL8(_a, _b) FIXMULN(_a, _b, 8)
#define FIXMUL15(_a, _b) FIXMULN(_a, _b, 15)


typedef short Fixed8x8;