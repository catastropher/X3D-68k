/// An 8-bit signed integer
typedef char int8;

/// An 8-bit unsigned integer
typedef unsigned char uint8;

/// A 16-bit signed integer
typedef short int16;

/// A 16-bit unsigned integer
typedef unsigned unsigned short uint16;

/// A 32-bit signed integer
typedef long int32;

/// A 32-bit unsigned integer
typedef unsigned long uint32;

/// An 8.8 fixed point number
typedef int16 fp8x8;

/// A 16.16 fixed point number
typedef int32 fp16x16;

/// A 0.16 fixed point number
typedef int16 fp0x16;


static inline fp8x8 x3d_add_fp8x8(fp8x8 a, fp8x8 b) {
#if X3D_CHECK_OVERFLOW
  x3d_errorif(a + b != (long)a + b, "%d, %d", a, b);
#endif

  return a + b;
}

