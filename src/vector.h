// This file is part of X3D.

// X3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// X3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with X3D. If not, see <http://www.gnu.org/licenses/>.



// Header File
// Created 4/19/2015; 4:40:24 PM

#pragma once
#include "fix.h"


//=============================================================================
// Defines
//=============================================================================

/// The number of fractional bits used to represent a normal in fixed point.
/// All normals are in 0.15 format.
#define NORMAL_BITS 15


//=============================================================================
// Structures
//=============================================================================

// Forward declarations
struct RenderContext;
struct Mat3x3;


/// A 3D vertex or vector with short values
typedef struct Vex3D {
	short x;			///< x component
	short y;			///< y component
	short z;			///< z component
} Vex3D;

/// A special padded 3D vector for the output of a 3D rotation
typedef struct{
	short x;			///< x component
	short pad1;			///< padding for x
	short y;			///< y component
	short pad2;			///< padding for y
	short z;			///< z component
	short pad3;			///< padding for z
} Vex3D_rot;

/// A 3D vertex or vector with unsigned char (small) values
typedef struct Vex3Ds {
	unsigned char x;	///< x component
	unsigned char y;	///< y component
	unsigned char z;	///< z component
} Vex3Ds;

/// A 3D vector with long values
typedef struct Vex3DL {
	long x;				///< x component
	long y;				///< y component
	long z;				///< z component
} Vex3DL;

/// A 2D vertex or vector
typedef struct {
	short x;			///< x component
	short y;			///< y component
} Vex2D;

//=============================================================================
// Functions
//=============================================================================
inline short dot_product(Vex3D* a, Vex3D* b);
void cross_product(Vex3D* a, Vex3D* b, Vex3D* dest);

inline void project_vex3d(struct RenderContext* rc, Vex3D* src, Vex2D* dest);

inline void sub_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest);
inline void add_vex3d(Vex3D* a, Vex3D* b, Vex3D* dest);

inline short get_vex3d_magnitude(Vex3D* v);
inline void normalize_vex3d(Vex3D* v);

void param_vex3d(Vex3D* start, Vex3D* end, Fixed8x8 t, Vex3D* dest);