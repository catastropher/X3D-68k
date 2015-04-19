// C Source File
// Created 4/19/2015; 6:13:35 PM

#include "geo/vector.h"

#include <tigcclib.h>

/**
 * Calculates the short dot product of two 3D vectors.
 *
 * @param a		- pointer to the first 3D vector
 * @param b		- pointer to the the second 3D vector
 *
 * @return dot product of a and b as a short
 * @note This returns the dot product shifted left by @ref NORMAL_BITS to fit in
 *      a short.
 */
inline short dot_product(Vex3D* a, Vex3D* b) {
	long prod = (long)a->x * b->x + (long)a->y * b->y + (long)a->z * b->z;
	
	return prod >> NORMAL_BITS;
}

/**
 * Calculates the dot product of two 3D vectors.
 *
 * @param a		- pointer to the first 3D vector
 * @param b		- pointer to the second 3D vector
 *
 * @return dot product of a and b as a long
 * @note This returns the unshifted dot product.
 */
long dot_product_long(Vex3D* a, Vex3D* b) {
	long prod = (long)a->x * b->x + (long)a->y * b->y + (long)a->z * b->z;
	
	return prod;
}

/**
 * Calculates the cross product of two 3D vectors. This creates a 3D vector
 *	that is perpendicular to both vectors. 
 *
 * @param a		- the first 3D vector
 * @param b		- the second 3D vector
 * @param dest	- pointer to the destination 3D vector
 *
 * @return cross product of a and b as a ref Vex3D
 * @note This routine will normalize the result (see ref normalize_vex3d).
 * 
 */
void cross_product(Vex3D* a, Vex3D* b, Vex3D* dest) {
	//dest->x = ((long)a->y * b->z - (long)a->z * b->y);
	//dest->y = ((long)a->z * b->x - (long)a->x * b->z);
	//dest->z = ((long)a->x * b->y - (long)a->y * b->x);
	
	
	//long long x = ((long long)a->y * b->z - (long long)a->z * b->y);
	//long long y = ((long long)a->z * b->x - (long long)a->x * b->z);
	//long long z = ((long long)a->x * b->y - (long long)a->y * b->x);
	
	
	//errorif(dest->x != x, "Invalid x");
	
	// FIXME: possible overflows in above code
	//long xx = x;
	
	long x_prod = (long)a->y * b->z;
	
	
	long xxx, yyy = 0, zzz = 0;
	
	
	//long xxx = ((((long)a->y * (short)b->z) >> 1) - (((long)a->z * b->y) >> 1));
	//long yyy = ((((long)a->z * b->x) >> 1) - (((long)a->x * b->z) >> 1));
	//long zzz = ((((long)a->x * b->y) >> 1) - (((long)a->y * b->x) >> 1));
	
	
	//long xxx = ((((long)a->y * (short)b->z) >> 1) - (((long)a->z * b->y) >> 1));
	{
		long ay_bz = (long)a->y * b->z;
		long az_by = (long)a->z * b->y;
		xxx = (ay_bz >> 1) - (az_by >> 1);
	}
	
	//long yyy = ((((long)a->z * b->x) >> 1) - (((long)a->x * b->z) >> 1));
	{
		long az_bx = (long)a->z * b->x;
		long ax_bz = (long)a->x * b->z;
		yyy = (az_bx >> 1) - (ax_bz >> 1);
	}
	
	//long zzz = ((((long)a->x * b->y) >> 1) - (((long)a->y * b->x) >> 1));
	//long ax_by = (long)a->x * b->y;
	//long ay_bx = (long)a->y * b->x;
	//zzz = (ax_by >> 1) - (ay_bx >> 1);
	
	
	
	
	//long yyy = ((long)a->z * b->x - (long)a->x * b->z);
	//long zzz = ((long)a->x * b->y - (long)a->y * b->x);
	
	//xassert(zzz == dest->z);
	
#if 0
	float res_z = ((float)a->x * b->y - (float)a->y * b->x) / 2;
	
	
	if(zzz != res_z) {
		printf("zzz: %ld, z: %f\n", zzz, res_z);
		ngetchx();
	}
	
	float res_y = ((float)a->z * b->x - (float)a->x * b->z) / 2.0;
	
	
	if(res_y != yyy) {
		printf("zzz: %ld, z: %f\n", zzz, res_z);
		ngetchx();
	}
	
	float res_x = ((float)a->y * b->z - (float)a->z * b->y) / 2;
	
	
	if(res_x != xxx) {
		printf("zzz: %ld, z: %f\n", zzz, res_z);
		ngetchx();
	}
	
	xassert(res_x == xxx);
	xassert(res_y == yyy);
	xassert(res_z == zzz);
	
#endif
	
	while(abs(xxx) >= 0x7FFF || abs(yyy) >= 0x7FFF || abs(zzz) >= 0x7FFF) {
		xxx >>= 1;
		yyy >>= 1;
		zzz >>= 1;
	}
	
	dest->x = xxx;
	dest->y = yyy;
	dest->z = zzz;
	
	//xassert(xxx == dest->x);
	//xassert(yyy == dest->y);
	
	
	
	//printf("Dest->z: %d\n", dest->z);
	
	normalize_vex3d(dest);
}