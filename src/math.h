// Header File
// Created 3/5/2015; 9:26:22 AM
#pragma once

#include "geo.h"

struct Vex3D;

short dot_product(struct Vex3D* a, struct Vex3D* b);
void cross_product(struct Vex3D* a, struct Vex3D* b, struct Vex3D* dest);