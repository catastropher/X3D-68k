//*****************************************************************************
/** \file extgraph.h
 * \brief This file contains definitions for the major part of the ExtGraph library (everything but the tilemap engine and preshifted sprites)
 *
 * ExtGraph is a compile-time library which contains <b>speed-optimized graphics
 * functions</b> for the TIGCC cross-compiler for TI-89, TI-89T, TI-92+ and TI-V200
 * (collectively known as TI-68k calculators).
 *
 * \version 2.00 Beta 6
 * \copyright Copyright (c) 2001-2008 TICT (TI-Chess Team) and contributors<br>
 * This library is maintained, improved and extended by:
 * <ul><li>Thomas Nussbaumer  (thomas.nussbaumer@gmx.net)</li>
 *     <li>Lionel Debroux     (lionel_debroux@yahoo.fr)</li>
 *     <li>Julien Richard-Foy a.k.a jachiechan / Sasume</li>
 *     <li>many contributors  (e.g. Geoffrey Anneheim a.k.a geogeo, many others)</li>
 * </ul>
 */
//*****************************************************************************
/* NOTES:
 *     * using ExtGraph 2.xx requires TIGCC 0.96 and newer (I'm nearly always
 *       using the latest version myself).
 *
 *     * ExtGraph (.h, .o and .a files in the lib/ folder of the distribution)
 *       can be used as is; recompiling ExtGraph is neither necessary nor
 *       recommended. Not to mention that it requires the GNU binutil
 *       m68k-coff-ar, which was removed from TIGCC a long time ago.
 *       The ExtGraph SVN repository contains PC/Win32 and i686-pc-linux-gnu
 *       m68k-coff-ar binaries. I can add binaries for other OS, starting with
 *       MacOS X, if someone provides me them.
 *
 *     * Please use as few __stkparm__ functions as possible in your programs:
 *       most functions have __regparm__ versions (or a mix of __regparm__ and
 *       __stkparm__: beyond about 6 parameters, some parameters are passed on
 *       the stack), which are themselves smaller and faster, and lead to
 *       smaller and faster code for passing arguments !
 *
 */
// ****************************************************************************

#ifndef __EXTGRAPH__
#define __EXTGRAPH__

#include <gray.h>   // necessary for GrayGetPlane() macros in the grayscale
                    // support routines !!
#include <stdlib.h> // min(a,b) macro.

// -----------------------------------------------------------------------------
// Reject TIGCC 0.95 and less. While the official release is still 0.95 as of
// the release of 2.00 Beta 5/6, it's outdated compared to 0.96 Beta 8.
// TIGCC 0.93 wouldn't handle calling conventions correctly; TIGCC 0.94 would
// make only small fusses, but the build process depends on some TIGCC 0.95+
// compiler/linker switches.
//
// Warn about other compilers because ExtGraph was tested only under TIGCC
// until further notice.
// -----------------------------------------------------------------------------
#include <version.h>

#ifndef __TIGCC__
#warning This compiler is unknown to ExtGraph. Until further notice, ExtGraph was tested only under TIGCC. Other compilers may not support ExtGraph.
#endif

#if (defined(__TIGCC__)&&(__TIGCC__<1)&&(defined(__TIGCC_MINOR__)&&(__TIGCC_MINOR__<96)))
#error ExtGraph requires TIGCC 0.96 or later ! Please update your TIGCC installation.
#endif


// -----------------------------------------------------------------------------
/** @defgroup extgraphversion Global variables indicating the ExtGraph version
 * To embed the extgraph version into your program, the ExtGraph library contains several global variables.<br>
 * They are embedded in the library to guarantee that the version is reported correctly even if you use an outdated header file (shame on you :-P !)
 */
// -----------------------------------------------------------------------------
//@{
#define EXTGRAPH_VERSION_STR      __egvs__    ///< This string looks like "ExtGraph vX.YY[<beta>n]" where X is the major version number and YY is the subversion number.
#define EXTGRAPH_VERSION_PWDSTR   __egvpwds__ ///< This string looks like "powered by ExtGraph vX.YY[<beta>n]" where X is the major version number and YY is the subversion number.
extern const char __egvs__[];                 ///< This string looks like "ExtGraph vX.YY[<beta>n]" where X is the major version number and YY is the subversion number.

extern const char __egvpwds__[];              ///< This string looks like "powered by ExtGraph vX.YY[<beta>n]" where X is the major version number and YY is the subversion number.

#define EXTGRAPH_VERSION_MAIN    __egvmain__  ///< This short represents the main ExtGraph version number (currently 2)
#define EXTGRAPH_VERSION_SUB     __egvsub__   ///< This short represents the ExtGraph sub-version number (currently 0)
#define EXTGRAPH_VERSION_REV     __egvrev__   ///< This short represents the ExtGraph revision number (currently 6)
extern const short __egvmain__;               ///< This short represents the main ExtGraph version number (currently 2)
extern const short __egvsub__;                ///< This short represents the ExtGraph sub-version number (currently 0)
extern const short __egvrev__;                ///< This short represents the ExtGraph revision number (currently 6)
//@}


// -----------------------------------------------------------------------------
// Enums used by some extgraph functions
// The values in enum GrayColors were changed in 2.00 to allow for more optimized grayscale utility functions.
// -----------------------------------------------------------------------------
//! Enum describing the values allowed for "color" parameters to the grayscale utility functions (e.g. \ref GrayDrawRect2B)
enum GrayColors {
    COLOR_WHITE = 0,
    COLOR_LIGHTGRAY = 1,
    COLOR_LGRAY = 1, ///< \since 2.00 Beta 5
    COLOR_DARKGRAY = 2,
    COLOR_DGRAY = 2, ///< \since 2.00 Beta 5
    COLOR_BLACK = 3
};
//! Enum describing the values allowed for "fill" parameters to the grayscale utility functions (e.g. \ref GrayDrawRect2B)
enum FillAttrs {
    RECT_EMPTY = 0, ///< Draw the rectangle outline only
    RECT_FILLED = 1 ///< Fill the rectangle
};
//! Enum describing the extended attributes handled by \ref GrayDrawStrExt2B
enum ExtAttrs {
    A_CENTERED = 0x40, ///< Draw centered
    A_SHADOWED = 0x80 ///< Draw lightgray shadow with (+1, +1) pixel offset
};


// -----------------------------------------------------------------------------
//! @defgroup pixmacros Fast pixel access macros
//! These macros are intended to be fast alternatives at the OS DrawPix function
//! \author Julien Richard-Foy, Kevin Kofler, Lionel Debroux, Sebastian Reichelt.
// The (address, bit number) variants were added early in the 2.00 beta series,
// in order to have GCC 3.x generate better code. GCC 4.x seems to do better
// with the old macros, but we will keep the new ones, which (at last !) work
// properly starting from 2.00 Beta 5, until we know GCC can do as well on its
// own with the old macros as it is forced to do by the new macros.
// -----------------------------------------------------------------------------
//@{
//! Offset in bytes of the (\a x, \a y) pixel from the (0, 0) pixel of a 240-pixel-wide plane.
#define EXT_PIXOFFSET(x, y) ((((y)<<4)-(y))*2+((x)>>3))
//! Address in memory of the (\a x, \a y) pixel of a 240-pixel-wide plane starting at \a p.
#define EXT_PIXADDR(p, x, y) (((char *)(p))+EXT_PIXOFFSET(x,y))
//! Pixel mask of coordinate \a x.
#define EXT_PIXMASK(x) (0x80>>((x)&7))
//! Bit number of coordinate \a x.
#define EXT_PIXNBIT(x) (~(x))


//! Internal macro: set pixel(s) at address \a a according to mask \a m
#define EXT_SETPIX_AM(a, m)   (*(a) |= (m))
//! Internal macro: clear pixel(s) at address \a a according to mask \a m
#define EXT_CLRPIX_AM(a, m)   (*(a) &= ~(m))
//! Internal macro: invert pixel(s) at address \a a according to mask \a m
#define EXT_XORPIX_AM(a, m)   (*(a) ^= (m))
//! Internal macro: get state of pixel(s) at address \a a, selected by mask \a m
#define EXT_GETPIX_AM(a, m)   (*(a) & (m))


//! Move one line up in a 240-pixel-wide plane
#define EXT_PIXUP(a, m) ((unsigned char *)(a) -= 30)
//! Move one line down in a 240-pixel-wide plane
#define EXT_PIXDOWN(a, m) ((unsigned char *)(a) += 30)
//! Shift pixel and address one pixel to the left, address/mask version
#define EXT_PIXLEFT_AM(a, m)   asm("rol.b  #1,%0;bcc.s  0f;subq.l #1,%1;0:"\
                                  : "=d" (m), "=g" (a) : "0" (m), "1" (a))
//! Shift pixel and address one pixel to the right, address/mask version
#define EXT_PIXRIGHT_AM(a, m)  asm("ror.b  #1,%0;bcc.s  0f;addq.l #1,%1;0:"\
                                  : "=d" (m), "=g" (a) : "0" (m), "1" (a))
//! Shift pixel and address one pixel to the left, address/bit number version
#define EXT_PIXLEFT_AN(a, n)   asm("addq.w #1,%0;cmpi.b #7,%0;bhi.s 0f;moveq #0,%0;subq.l #1,%1;0:"\
                                  : "=d" (n), "=g" (a) : "0" (m), "1" (a))
//! Shift pixel and address one pixel to the right, address/bit number version
#define EXT_PIXRIGHT_AN(a, n)  asm("subq.w #1,%0;blt.s  0f;moveq #7,%0;addq.l #1,%1;0:"\
                                  : "=d" (n), "=g" (a) : "0" (m), "1" (a))


// These macros are more optimized than EXT_..._AM ones, but less readable...
// Thanks a lot to Sebastian for giving me the solution to the trigraph
// problem, I couldn't think of escaping one of the offending question marks !
//! Internal macro: set pixel whose bit number is \a n, at offset \a offset from a 240-pixel-wide plane starting at \a a
#define EXT_SETPIX_AN(a, offset, n) ({if(__builtin_constant_p(offset)) \
	{	\
		if(__builtin_constant_p(a))	\
			asm("bset.b %0,%c1" : : "di" (n), "dai" ((offset)+(unsigned char*)(a)));	\
		else	\
			if (offset !=0) \
				asm("bset.b %0,%c1(%2)" : : "di,di" (n), "i,\?\?\?\?!g" (offset), "a,a" (a));	\
			else \
				asm("bset.b %0,(%1)" : : "di" (n), "a" (a));	\
	}	\
	else	\
		asm("bset.b %0,0(%2,%1.w)" : : "di" (n), "da" (offset), "a" (a));	\
	})

//! Internal macro: clear pixel whose bit number is \a n, at offset \a offset from a 240-pixel-wide plane starting at \a a
#define EXT_CLRPIX_AN(a, offset, n) ({if(__builtin_constant_p(offset)) \
	{	\
		if(__builtin_constant_p(a))	\
			asm("bclr.b %0,%c1" : : "di" (n), "dai" ((offset)+(unsigned char*)(a)));	\
		else	\
			if (offset !=0) \
				asm("bclr.b %0,%c1(%2)" : : "di,di" (n), "i,\?\?\?\?!g" (offset), "a,a" (a));	\
			else \
				asm("bclr.b %0,(%1)" : : "di" (n), "a" (a));	\
	}	\
	else	\
		asm("bclr.b %0,0(%2,%1.w)" : : "di" (n), "da" (offset), "a" (a));	\
	})

//! Internal macro: invert pixel whose bit number is \a n, at offset \a offset from a 240-pixel-wide plane starting at \a a
#define EXT_XORPIX_AN(a, offset, n) ({if(__builtin_constant_p(offset)) \
	{ \
		if(__builtin_constant_p(a)) \
			asm("bchg.b %0,%c1" : : "di" (n), "dai" ((offset)+(unsigned char*)(a))); \
		else \
			if (offset !=0) \
				asm("bchg.b %0,%c1(%2)" : : "di,di" (n), "i,\?\?\?\?!g" (offset), "a,a" (a)); \
			else \
				asm("bchg.b %0,(%1)" : : "di" (n), "a" (a)); \
	} \
	else \
		asm("bchg.b %0,0(%2,%1.w)" : : "di" (n), "da" (offset), "a" (a)); \
	})


//! Internal macro: get state of pixel whose bit number is \a n, at offset \a offset from a 240-pixel-wide plane starting at \a a
#define EXT_GETPIX_AN(a, offset, n) ({char __result; if(__builtin_constant_p(offset)) \
	{ \
		if(__builtin_constant_p(a)) \
			asm("btst.b %1,%c2; sne.b %0" : "=d" (__result) : "di" (n), "dai" ((offset)+(unsigned char*)(a))); \
		else \
			if (offset !=0) \
				asm("btst.b %1,%c2(%3); sne.b %0" : "=d,d" (__result) : "di,di" (n), "i,\?\?\?\?!g" (offset), "a,a" (a)); \
			else \
				asm("btst.b %1,(%2); sne.b %0" : "=d" (__result) : "di" (n), "a" (a)); \
	} \
	else \
		asm("btst.b %1,0(%3,%2.w); sne.b %0" : "=d" (__result) : "di" (n), "da" (offset), "a" (a)); \
	__result;})


//! Set pixel at coordinates (\a x, \a y) in 240-pixel-wide plane starting at \a p.
#define EXT_SETPIX(p, x, y) EXT_SETPIX_AN(p, EXT_PIXOFFSET(x,y), EXT_PIXNBIT(x))
//! Clear pixel at coordinates (\a x, \a y) in 240-pixel-wide plane starting at \a p.
#define EXT_CLRPIX(p, x, y) EXT_CLRPIX_AN(p, EXT_PIXOFFSET(x,y), EXT_PIXNBIT(x))
//! Invert pixel at coordinates (\a x, \a y) in 240-pixel-wide plane starting at \a p.
#define EXT_XORPIX(p, x, y) EXT_XORPIX_AN(p, EXT_PIXOFFSET(x,y), EXT_PIXNBIT(x))
//! Get state of pixel at coordinates (\a x, \a y) in 240-pixel-wide plane starting at \a p.
#define EXT_GETPIX(p, x, y) EXT_GETPIX_AN(p, EXT_PIXOFFSET(x,y), EXT_PIXNBIT(x))
//@}



// -----------------------------------------------------------------------------
//! @defgroup miscmacros Miscellanous macros
// -----------------------------------------------------------------------------
//@{
#ifndef __HAVE_DEREFSMALL
#define __HAVE_DEREFSMALL
//! Dereferences a pointer: DEREFSMALL(\a p,\a i) does the same as <code>p[i]</code>, but in a faster and smaller way.
// Doing the same thing using inline assembly saved ~100 bytes on an internal, buggy version of tthdex.
// Credits go to Kevin Kofler for its generic definition and the &* trick.
// 2.00 Beta 5: added ifndef/define pair so as to minimize incompatibility chances with the (same)
// definition that could be added to TIGCC some day.
#define DEREFSMALL(__p, __i) (*((typeof(&*(__p)))((unsigned char*)(__p)+(long)(short)((short)(__i)*sizeof(*(__p))))))
#endif

//! Returns the absolute value of given short
#define EXT_SHORTABS(a)  ({register short __ta=(a); (__ta>=0) ? __ta : -__ta;})

//! Returns the absolute value of given long
#define EXT_LONGABS(a)  ({register long __ta=(a); (__ta>=0) ? __ta : -__ta;})

//! Exchanges the content of two variables using the ASM exg instruction
// 2.00 adds "a" to the constraints, since exg applies also to address registers.
#define EXT_XCHG(a, b)    asm volatile ("exg %0,%1" : "=da" (a), "=da" (b) : "0" (a), "1" (b) : "cc")

//! Returns the word swapped value (upper and lower word swapped) of the given long
#define EXT_LONGSWAP(val) ({register unsigned long __tmp = val;asm volatile ("swap %0" : "=d" (__tmp) : "0" (__tmp));__tmp;})
//@}


// -----------------------------------------------------------------------------
//! @defgroup boundcollidemacros Bounds collision macros
//! Check two bounding rectangles whose vertices are (\a x0, \a y0), (\a x1, \a y0), (\a x0, \a y1), and (\a x1, \a y1) for collision.
//! '\a w' parameters are widths in pixels, '\a h' parameters are heights in pixels.
//@{
#define BOUNDS_COLLIDE(x0, y0, x1, y1, w, h) \
   (((EXT_SHORTABS((x1)-(x0)))<(w))&&((EXT_SHORTABS((y1)-(y0)))<(h)))

#define BOUNDS_COLLIDE2H(x0, y0, x1, y1, w, h0, h1) \
   (((EXT_SHORTABS((x1)-(x0)))<(w)) && ((EXT_SHORTABS((y1)-(y0)))<(min((h0),(h1))))) ///< This macro can handle two bounding rectangles of different heights 

#define BOUNDS_COLLIDE2W(x0, y0, x1, y1, w0, w1, h) \
   (((EXT_SHORTABS((x1)-(x0)))<(min((w0),(w1)))) && ((EXT_SHORTABS((y1)-(y0)))<(h))) ///<  This macro can handle two bounding rectangles of different widths 

#define BOUNDS_COLLIDE2HW(x0, y0, x1, y1, w0, w1, h0, h1) \
   (((EXT_SHORTABS((x1)-(x0)))<(min((w0),(w1)))) && ((EXT_SHORTABS((y1)-(y0)))<(min((h0),(h1))))) ///<  This macro can handle two bounding rectangles of different widths and heights 
//@}

// -----------------------------------------------------------------------------
//! @defgroup boundcollideshortcuts Shortcuts for bounds collision macros
//! @ingroup boundcollidemacros
//! Shortcuts for \ref BOUNDS_COLLIDE with standard common tile sizes (8x8 / 8xh / 16x16 / 16xh / 32x32 / 32xh).
// -----------------------------------------------------------------------------
//@{
#define BOUNDS_COLLIDE8(x0, y0, x1, y1)  BOUNDS_COLLIDE(x0, y0, x1, y1, 8, 8)
#define BOUNDS_COLLIDE16(x0, y0, x1, y1) BOUNDS_COLLIDE(x0, y0, x1, y1, 16, 16)
#define BOUNDS_COLLIDE32(x0, y0, x1, y1) BOUNDS_COLLIDE(x0, y0, x1, y1, 32, 32)
#define BOUNDS_COLLIDE82H(x0, y0, x1, y1, h0, h1)  BOUNDS_COLLIDE2H(x0, y0, x1, y1, 8, h0, h1)
#define BOUNDS_COLLIDE162H(x0, y0, x1, y1, h0, h1) BOUNDS_COLLIDE2H(x0, y0, x1, y1, 16, h0, h1)
#define BOUNDS_COLLIDE322H(x0, y0, x1, y1, h0, h1) BOUNDS_COLLIDE2H(x0, y0, x1, y1, 32, h0, h1)
//@}



//--BEGIN_FUNCTION_PROTOTYPES--//

// -----------------------------------------------------------------------------
/** @defgroup testcollide Sprite/sprite collision test functions
 * \brief Check for collision between 2 sprites of various widths (depending on the function's name), the top-left corner of first one being at (\a x0, \a y0) and that of the other one at (\a x1, \a y1).
 *
 * These functions can test irregulary shaped sprites (not only rectangular ones) for collision, by shifting the content if necessary and "AND"-ing the data together: if a set pixel overlaps (i.e. is set in both sprites) a collision is detected.
 * "2h" variants can handle two sprites of different heights.
 *
 * \note For grayscale sprites it is a good idea to use a special type of mask data for \a data0 and \a data1, where every pixel
 * which should be involved in testing is set. A simple way to generate such a mask is to OR the data of the dark plane and the light plane
 * together into one plane (see \ref SpriteX8Data_withsprite_OR_R ). If you use such a mask you'll need to call TestCollide8 only once.
 *
 * \return 0 if the sprites don't collide, nonzero otherwise.
 */
// -----------------------------------------------------------------------------
//@{
short TestCollide8(short x0, short y0, short x1, short y1, unsigned short height, const unsigned char* data0, const unsigned char* data1) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
short TestCollide8_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height, const unsigned char* data0 asm("%a0"), const unsigned char* data1 asm("%a1")) __attribute__((__stkparm__));
short TestCollide82h_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height0, unsigned short height1, const unsigned char* data0 asm("%a0"), const unsigned char* data1 asm("%a1")) __attribute__((__stkparm__)); ///< This function can handle two sprites of different heights \since 2.00 Beta 5

short TestCollide16(short x0, short y0, short x1, short y1, unsigned short height, const unsigned short* data0, const unsigned short* data1) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
short TestCollide16_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height, const unsigned short* data0 asm("%a0"), const unsigned short* data1 asm("%a1")) __attribute__((__stkparm__));
short TestCollide162h_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height0, unsigned short height1, const unsigned short* data0 asm("%a0"), const unsigned short* data1 asm("%a1")) __attribute__((__stkparm__)); ///< This function can handle two sprites of different heights \since 2.00 Beta 5

short TestCollide322h_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height0, unsigned short height1, const unsigned long *data0 asm("%a0"), const unsigned long *data1 asm("%a1")) __attribute__((__stkparm__)); ///< This function can handle two sprites of different heights \since 2.00 Beta 5 \author Joey Adams

short TestCollideX82w2h_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1, unsigned short bytewidth0, unsigned short bytewidth1, unsigned short height0, unsigned short height1, const void *data0 asm("%a0"), const void *data1 asm("%a1")) __attribute__((__stkparm__)); ///< This function can handle two sprites of different heights \since 2.00 Beta 5 \author Joey Adams
short TestCollideX82w2h_invsprts_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1, unsigned short bytewidth0, unsigned short bytewidth1, unsigned short height0, unsigned short height1, const void *data0 asm("%a0"), const void *data1 asm("%a1")) __attribute__((__stkparm__)); ///< This function can handle two sprites of different heights \since 2.00 Beta 6 \author Joey Adams
//@}



// -----------------------------------------------------------------------------
/** @defgroup pixcollide Pixel/sprite collision test functions
 * \brief Check for collision between pixel at (\a x0, \a y0) and sprites of various widths (depending on the function's name) whose top-left corner is at (\a x1, \a y1).
 *
 * \since 2.00 Beta 5
 * \author Joey Adams, Samuel Stearley, Jesse Frey, Lionel Debroux.
 */
// -----------------------------------------------------------------------------
//@{
char PixCollide8_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height asm("%a1"), const unsigned char *sprite asm("%a0")) __attribute__((__regparm__(6)));
char PixCollide16_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height asm("%a1"), const unsigned short *sprite asm("%a0")) __attribute__((__regparm__(6)));
char PixCollide32_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short height asm("%a1"), const unsigned long *sprite asm("%a0")) __attribute__((__regparm__(6)));
char PixCollideX8_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short bytewidth asm("%d4"), short height asm("%a1"), const unsigned char *sprite asm("%a0")) __attribute__((__regparm__));
char PixCollideX16_R(short x0 asm("%d0"), short y0 asm("%d1"), short x1 asm("%d2"), short y1 asm("%d3"), unsigned short wordwidth asm("%d4"), short height asm("%a1"), const unsigned short *sprite asm("%a0")) __attribute__((__regparm__));
//@}


// -----------------------------------------------------------------------------
//! @defgroup grayutils Grayscale utility functions
//! \note Most of these functions modify the active plane using PortSet() !

//! @defgroup grayutilfuncs Grayscale utility functions
//! @ingroup grayutils
//! \note Most of these functions modify the active plane with PortSet() !
// -----------------------------------------------------------------------------
//@{
//! Clear the given 240x128 planes
//! \note \a lightplane and \a darkplane must start at an even address.
void GrayClearScreen2B(void* lightplane, void* darkplane) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function

//! Clear the given 240x128 planes, register-parameter-passing version.
//! \note \a lightplane and \a darkplane must start at an even address.
void GrayClearScreen2B_R(void* lightplane asm("%a0"), void* darkplane asm("%a1")) __attribute__((__regparm__(2)));

//! Fill the given 240x128 planes with given 32-bit patterns.
//! \note \a lightplane and \a darkplane must start at an even address.
void GrayFillScreen2B_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned long lcolor asm("%d0"), unsigned long dcolor asm("%d1")) __attribute__((__regparm__(4)));

//! Draw to the given 240x128 planes the rectangle whose vertices are (\a x0, \a y0), (\a x1, \a y0), (\a x0, \a y1) and (\a x1, \a y1).<br>
//! color is an element of \ref GrayColors, fill is an element of \ref FillAttrs.
void GrayDrawRect2B(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, short color, short fill, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Invert the rectangle whose whose vertices are (\a x0, \a y0), (\a x1, \a y0), (\a x0, \a y1) and (\a x1, \a y1) in both given 240x128 planes.
void GrayInvertRect2B(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Draw the line between (x0, y0) and (x1, y1) in both 240x128 planes given, using the OS DrawLine routine.<br>
//! color is an element of enum \ref GrayColors.
void GrayDrawLine2B(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, short color, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Draw the line between (x0, y0) and (x1, y1) in both 240x128 planes given, using the OS DrawClipLine routine.<br>
//! color is an element of enum \ref GrayColors.
//! \since 2.00 Beta 6
void GrayDrawClipLine2B(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, short color, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Draw the line between (x0, y0) and (x1, y1) in both 240x128 planes given, using the ExtGraph FastDrawLine routine.<br>
//! color is an element of enum \ref GrayColors.
void GrayFastDrawLine2B(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, short color, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Draw the line between (x0, y) and (x1, y) in both 240x128 planes given, using the ExtGraph FastDrawHLine routine.<br>
//! color is an element of enum \ref GrayColors.
void GrayFastDrawHLine2B(unsigned short x0, unsigned short x1, unsigned short y, short color, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Draw the character c using drawing mode attr between (x0, y) and (x1, y) in both 240x128 planes given, using the OS DrawChar routine.
void GrayDrawChar2B(unsigned short x, unsigned short y, char c, short attr, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Draw the string s using drawing mode attr between (x0, y) and (x1, y) in both 240x128 planes given, using the OS DrawStr routine.
void GrayDrawStr2B(unsigned short x, unsigned short y, const char* s, short attr, void* lightplane, void* darkplane) __attribute__((__stkparm__));

//! Draw the string s using drawing mode attr between (x0, y) and (x1, y) in both 240x128 planes given, using the OS DrawStr routine.<br>
//! attr can be ORed with the values of enum \ref ExtAttrs for centered drawing and/or drawing with a lighter shadow on the bottom right of the string
void GrayDrawStrExt2B(unsigned short x, unsigned short y, const char* s, short attr, short font, void* lightplane, void* darkplane) __attribute__((__stkparm__));
//@}

//! @defgroup grayutildeprecated Grayscale utility functions, deprecated names
//! @ingroup grayutils
//! These names are not compliant with the modern names of the grayscale functions of TIGCCLIB.
//@{
#define ClearGrayScreen2B GrayClearScreen2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define ClearGrayScreen2B_R GrayClearScreen2B_R ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayRect2B GrayDrawRect2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define InvertGrayRect2B GrayInvertRect2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayLine2B GrayDrawLine2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define FastDrawGrayLine2B GrayFastDrawLine2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define FastDrawGrayHLine2B GrayFastDrawHLine2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayChar2B GrayDrawChar2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayStr2B GrayDrawStr2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayStrExt2B GrayDrawStrExt2B ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
//@}


// -----------------------------------------------------------------------------
/** @defgroup grayutilmacros Shortcut macros for the grayscale utility functions
 * @ingroup grayutils
 * \brief These macros are shortcuts for the ExtGraph function that starts with the same name, passing them the standard planes of the TIGCCLIB grayscale support, i.e. GrayGetPlane(LIGHT_PLANE) and GrayGetPlane(DARK_PLANE).
 * \note Most of these functions modify the active plane with PortSet() !
 */
// -----------------------------------------------------------------------------
//@{
#define GrayClearScreen()                         GrayClearScreen2B(GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayClearScreen2B--
#define GrayClearScreen_R()                       GrayClearScreen2B_R(GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayClearScreen2B_R--
#define GrayFillScreen_R(l, d)                    GrayFillScreen2B_R(GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE), l, d) // --TESTS-GrayFillScreen2B_R--
#define GrayDrawRect(x0, y0, x1, y1, color, fill) GrayDrawRect2B(x0, y0, x1, y1, color, fill, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayDrawRect2B--
#define GrayInvertRect(x0, y0, x1, y1)            GrayInvertRect2B(x0, y0, x1, y1, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayInvertRect2B--
#define GrayDrawLine(x0, y0, x1, y1, color)       GrayDrawLine2B(x0, y0, x1, y1, color, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayDrawLine2B--
//! \since 2.00 Beta 6
#define GrayDrawClipLine(x0, y0, x1, y1, color)   GrayDrawClipLine2B(x0, y0, x1, y1, color, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayDrawClipLine2B--

#define GrayFastDrawHLine(x0, x1, y, color)       GrayFastDrawHLine2B(x0, x1, y, color, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayFastDrawHLine2B--
#define GrayFastDrawLine(x0, y0, x1, y1, color)   GrayFastDrawLine2B(x0, y0, x1, y1, color, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayFastDrawLine2B--
#define GrayDrawChar(x, y, c, attr)               GrayDrawChar2B(x, y, c, attr, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayDrawChar2B--
#define GrayDrawStr(x, y, s, attr)                GrayDrawStr2B(x, y, s, attr, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayDrawStr2B--
#define GrayDrawStrExt(x, y, s, attr, font)       GrayDrawStrExt2B(x, y, s, attr, font, GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE)) // --TESTS-GrayDrawStrExt2B--
//@}

//! @defgroup grayutilmacrosdeprecated Shortcut macros for the grayscale utility functions, deprecated names
//! @ingroup grayutils
//@{
#define ClearGrayScreen GrayClearScreen ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define ClearGrayScreen_R GrayClearScreen_R ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayRect GrayDrawRect ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define InvertGrayRect GrayInvertRect ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayLine GrayDrawLine ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define FastDrawGrayHLine GrayFastDrawHLine ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define FastDrawGrayLine GrayFastDrawLine ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayChar GrayDrawChar ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayStr GrayDrawStr ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
#define DrawGrayStrExt GrayDrawStrExt ///< \deprecated Name not compliant with the modern names of the grayscale functions of TIGCCLIB.
//@}



// -----------------------------------------------------------------------------
//! @defgroup scrolling Screen scrolling functions 
//! These functions 160x\a lines or 240x\a lines pixels of a 240-pixel-wide plane pointed to by \a buffer, 1 pixel at a time
//! \todo add n-pixel-at-a-time scrolling routine (like that made by Scott Noveck) ?
// -----------------------------------------------------------------------------
//@{
void ScrollLeft160(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void ScrollLeft240(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void ScrollRight160(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void ScrollRight240(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void ScrollUp160(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void ScrollUp240(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void ScrollDown160(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void ScrollDown240(unsigned short* buffer, unsigned short lines) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function

void ScrollLeft160_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
void ScrollLeft240_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
void ScrollRight160_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
void ScrollRight240_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
void ScrollUp160_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
void ScrollUp240_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
void ScrollDown160_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
void ScrollDown240_R(unsigned short* buffer asm("%a0"), unsigned short lines asm("%d0")) __attribute__((__regparm__(2)));
//@}



// -----------------------------------------------------------------------------
/** @defgroup line Line functions
 * \brief Fast functions for line drawing.
 */
// -----------------------------------------------------------------------------
/** @defgroup genericline Generic line drawing routines, multiple drawing modes
 * @ingroup line
 * \brief These routines draw a line from (\a x1, \a y1) to (\a x2, \a y2) in one or two 240-pixel-wide video plane(s) using attribute \a mode or color \a color.
 *
 * Valid modes are:
 * <ul><li>A_REVERSE, A_NORMAL, A_XOR, A_REPLACE, A_OR for \ref FastDrawLine and \ref FastDrawLine_R (actually, A_NORMAL = A_REPLACE = A_OR is assumed if mode is neither A_REVERSE nor A_XOR)</li>
 * <li>the elements of enum \ref GrayColors for \ref GrayFastDrawLine2B_R</li></ul>
 */
//@{
void FastDrawLine(void* plane, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, short mode) __attribute__((__stkparm__)); ///< \author Olivier Armand, Lionel Debroux \deprecated __stkparm__ function with equivalent __regparm__ function
void FastDrawLine_R(void* plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short mode) __attribute__((__stkparm__)); ///< \author Olivier Armand, Lionel Debroux 
void GrayFastDrawLine2B_R(void* plane0 asm("%a0"), void *plane1 asm("%a1"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short color) __attribute__((__stkparm__)); ///< \author Olivier Armand, Lionel Debroux \since 2.00 Beta 5
//@}

/** @defgroup genericlinesingle Generic line drawing routines, hard-coded drawing mode
 * @ingroup line
 * \brief These routines draw a line from (\a x1, \a y1) to (\a x2, \a y2) in a 240-pixel-wide video plane pointed to by \a plane, with hard-coded drawing mode.
 */
//@{
void FastLine_Draw_R(void *plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__)); ///< \author Julien Richard-Foy
void FastLine_Erase_R(void *plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__)); ///< \author Julien Richard-Foy
void FastLine_Invert_R(void *plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__)); ///< \author Julien Richard-Foy
//@}

/** @defgroup horzline Horizontal line drawing routines, multiple drawing modes
 * @ingroup line
 * \brief These routines draw a line from (\a x1, \a y) to (\a x2, \a y) in one or two 240-pixel-wide video plane(s) using attribute \a mode or color \a color.
 *
 * Valid modes are:
 * <ul><li>A_REVERSE, A_NORMAL, A_XOR, A_REPLACE, A_OR for \ref FastDrawHLine and \ref FastDrawHLine_R (actually, A_NORMAL = A_REPLACE = A_OR is assumed if mode is neither A_REVERSE nor A_XOR)</li>
 * <li>the elements of enum \ref GrayColors for \ref GrayFastDrawHLine2B_R</li></ul>
 */
//@{
void FastDrawHLine(void* plane, unsigned short x1, unsigned short x2, unsigned short y, short mode) __attribute__((__stkparm__)); ///< Draw horizontal line \deprecated __stkparm__ function with equivalent __regparm__ function
void FastDrawHLine_R(void* plane asm("a0"), unsigned short x1 asm("d0"), unsigned short x2 asm("d1"), unsigned short y asm("d2"), short mode) __attribute__((__stkparm__));
void GrayFastDrawHLine2B_R(void *plane0 asm("%a0"), void *plane1 asm("%a1"), unsigned short x1 asm("%d0"), unsigned short x2 asm("%d1"), unsigned short y asm("%d2"), short color asm("%d3")); ///< \since 2.00 Beta 5
//@}

/** @defgroup vertline Vertical line drawing routines, multiple drawing modes
 * @ingroup line
 * \brief These routines draw a line from (\a x, \a y1) to (\a x, \a y2) in a 240-pixel-wide video plane pointed to by \a plane using attribute \a mode. 
 *
 * Valid modes are A_REVERSE, A_NORMAL, A_XOR, A_REPLACE, A_OR (actually, A_NORMAL = A_REPLACE = A_OR is assumed if mode is neither A_REVERSE nor A_XOR).
 * \todo GrayFastDrawVLine2B_R
 */
//@{
void FastDrawVLine(void* plane, unsigned short x, unsigned short y1, unsigned short y2, short mode) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void FastDrawVLine_R(void* plane asm("%a0"), unsigned short x asm("%d0"), unsigned short y1 asm("%d1"), unsigned short y2 asm("%d2"), short mode) __attribute__((__stkparm__));
//@}


/** @defgroup testline Plane/line collision test
 * @ingroup line
 * \brief These routines check whether there's any pixel set on the line that goes from (\a x1, \a y1) to (\a x2, \a y2) in a 240-pixel-wide video plane pointed to by \a plane. 
 *
 * \ref FastTestLine_BE_R tests both ends at the same time, \ref FastTestLine_LE_R starts with the left end, and \ref FastTestLine_RE_R starts with the right end.
 * \since 2.00 Beta 5
 */
//@{
char FastTestLine_BE_R(void *plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__));
char FastTestLine_LE_R(void *plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__));
char FastTestLine_RE_R(void *plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__));
//@}

/** @defgroup clipline Line clipping
 * @ingroup line
 * \brief These routines clip the line that goes from (\a x1, \a y1) to (\a x2, \a y2) to the boundaries of a 240x128 plane, and optionally draw it using a callback to a routine compatible with \ref FastDrawLine_R or \ref GrayFastDrawLine2B_R.
 *
 * Valid drawing modes are:
 * <ul><li>A_REVERSE, A_NORMAL, A_XOR, A_REPLACE, A_OR for \ref ClipDrawLine_R (actually, A_NORMAL = A_REPLACE = A_OR is assumed if mode is neither A_REVERSE nor A_XOR)</li>
 * <li>the elements of enum \ref GrayColors for \ref GrayClipDrawLine_R</li></ul>
 *
 * \warning These routines performs no checking on the value of a1. If you pass a1 = 0, you'll get "Protected Memory Violation" (but in C, you do want to use this parameter to get the clipped coordinates).
 * \note In assembly, you can use _ClipLine_R as follows:
 * <ul><li>arguments: x1 -> d0, y1 -> d1, x2 -> d2, y2 -> d3.</li>
 * <li>return value: clipped coordinates in d0-d3, 0 in a0 if nothing to draw (nonzero otherwise).</li></ul>
 *
 * \since 2.00 Beta 5
 */
//@{
void * ClipLine_R(short x1 asm("%d0"), short y1 asm("%d1"), short x2 asm("%d2"), short y2 asm("%d3"), unsigned short *clippedcoord asm("%a1")) __attribute__((__regparm__)); ///< \return a0 = NULL if there's nothing to draw.
// 
void ClipDrawLine_R(short x1 asm("%d0"), short y1 asm("%d1"), short x2 asm("%d2"), short y2 asm("%d3"), unsigned short *clippedcoord asm("%a1"), short mode, void *plane asm("%a0"), void (__attribute__((__stkparm__)) *)(void* plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short mode)) __attribute__((__stkparm__)); // --TESTS-ClipLine_R--
void GrayClipDrawLine_R(short x1 asm("%d0"), short y1 asm("%d1"), short x2 asm("%d2"), short y2 asm("%d3"), unsigned short *clippedcoord asm("%a1"), short color, void *plane0, void *plane, void (__attribute__((__stkparm__)) *)(void* plane0 asm("%a0"), void *plane1 asm("%a1"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short color)) __attribute__((__stkparm__)); // --TESTS-ClipLine_R--
//@}



// -----------------------------------------------------------------------------
//! @defgroup rectangle Rectangle drawing functions
//! Functions for rectangle drawing. Much faster than the AMS functions.
// -----------------------------------------------------------------------------
/** @defgroup fillrect Rectangle filling functions, multiple drawing modes
 * @ingroup rectangle
 * \brief These routines fill a rectangle whose vertices are (\a x1, \a y1), (\a x2, \a y1), (\a x1, \a y2) and (\a x2, \a y2) in one or two 240-pixel-wide video plane(s) using attribute \a mode or color \a color. 
 *
 * Valid modes are:
 * <ul><li>A_REVERSE, A_NORMAL, A_XOR, A_REPLACE, A_OR for \ref FastDrawLine and \ref FastDrawLine_R (actually, A_NORMAL = A_REPLACE = A_OR is assumed if mode is neither A_REVERSE nor A_XOR)</li>
 * <li>the elements of enum \ref GrayColors for GrayFastDrawLine2B_R</li></ul>
 */
//@{
void FastFillRect(void* plane, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, short mode) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void FastFillRect_R(void* plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short mode) __attribute__((__stkparm__));
void GrayFastFillRect_R(void* dest0 asm("%a0"), void* dest1 asm("%a1"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short color) __attribute__((__stkparm__)); // --TESTS-FastFillRect_R--
//@}

/** @defgroup fillrectsingle Rectangle filling functions, hard-coded drawing mode
 * @ingroup rectangle
 * \brief These routines fill a rectangle whose vertices are (\a x1, \a y1), (\a x2, \a y1), (\a x1, \a y2) and (\a x2, \a y2) in a 240-pixel-wide video plane pointed to by \a plane, with hard-coded drawing mode.
 */
//@{
void FastFilledRect_Draw_R(void* plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__(5)));
void FastFilledRect_Erase_R(void* plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__(5)));
void FastFilledRect_Invert_R(void* plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3")) __attribute__((__regparm__(5)));
//@}

/** @defgroup outlinerect Rectangle outlining functions, multiple drawing modes
 * @ingroup rectangle
 * \brief These routines draw the outline of a rectangle, i.e. its vertices (\a x1, \a y1), (\a x2, \a y1), (\a x1, \a y2) and (\a x2, \a y2), in one or two 240-pixel-wide video plane(s) using attribute \a mode or color \a color. 
 *
 * Valid modes are:
 * <ul><li>A_REVERSE, A_NORMAL, A_XOR, A_REPLACE, A_OR for \ref FastDrawLine and \ref FastDrawLine_R (actually, A_NORMAL = A_REPLACE = A_OR is assumed if mode is neither A_REVERSE nor A_XOR)</li>
 * <li>the elements of enum \ref GrayColors for GrayFastDrawLine2B_R</li></ul>
 */
//@{
//! \deprecated __stkparm__ function with equivalent __regparm__ function
void FastOutlineRect(void* plane, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, short mode) __attribute__((__stkparm__)); // --TESTS-FastDrawHLine,FastDrawVLine--
void FastOutlineRect_R(void* plane asm("%a0"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short mode) __attribute__((__stkparm__)); // --TESTS-FastDrawHLine_R,FastDrawVLine_R--
void GrayFastOutlineRect_R(void* dest0 asm("%a0"), void* dest1 asm("%a1"), unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), short color) __attribute__((__stkparm__)); // --TESTS-FastDrawHLine_R,FastDrawVLine_R--
//@}

/** @defgroup fastrect Fast rectangle filling functions for screen-wide widths
 * @ingroup rectangle
 * \brief These routines fill \a lines lines of the (160|240)-pixel-wide rectangle whose topmost line is at \a starty, in one or two 240x128 video plane(s), with hard-coded drawing mode.<br>
 *
 */
//@{
void FastEraseRect160_R(void* plane asm("%a0"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(3)));
void FastEraseRect240_R(void* plane asm("%a0"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(3)));
void FastFillRect160_R(void* plane asm("%a0"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(3)));
void FastFillRect240_R(void* plane asm("%a0"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(3)));
void FastInvertRect160_R(void* plane asm("%a0"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(3)));
void FastInvertRect240_R(void* plane asm("%a0"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(3)));

void GrayFastEraseRect2B160_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(4)));
void GrayFastEraseRect2B240_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(4)));
void GrayFastFillRect2B160_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(4)));
void GrayFastFillRect2B240_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(4)));
void GrayFastInvertRect2B160_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(4)));
void GrayFastInvertRect2B240_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short starty asm("%d0"), unsigned short lines asm("%d1")) __attribute__((__regparm__(4)));
//@}

/** @defgroup fastrectx8 Fast rectangle filling functions for widths multiple of 8
 * @ingroup rectangle
 * \brief These routines fill \a lines lines of the (\a bytewidth *8)-pixel-wide rectangle whose top-left corner is at (\a startcol *8, \a starty), in a 240-pixel-wide video plane pointed to by \a plane, with hard-coded drawing mode.<br>
 */
//@{
void FastEraseRectX8_R(void* plane asm("%a0"), unsigned short startcol asm("%d0"), unsigned short starty asm("%d1"), unsigned short lines asm("%d2"), unsigned short bytewidth asm("%d3")) __attribute__((__regparm__(5)));
void FastFillRectX8_R(void* plane asm("%a0"), unsigned short startcol asm("%d0"), unsigned short starty asm("%d1"), unsigned short lines asm("%d2"), unsigned short bytewidth asm("%d3")) __attribute__((__regparm__(5)));
void FastInvertRectX8_R(void* plane asm("%a0"), unsigned short startcol asm("%d0"), unsigned short starty asm("%d1"), unsigned short lines asm("%d2"), unsigned short bytewidth asm("%d3")) __attribute__((__regparm__(5)));
//@}




// -----------------------------------------------------------------------------
/** @defgroup circle Circle drawing functions
 * \brief Fast functions for circle drawing.
 *
 * These functions are much faster than the OS DrawClipEllipse function, but
 * that's partly due to the fact DrawClipEllipse supports multiple drawing modes
 * and can draw ellipses, not just circles. Therefore, any bench between
 * DrawClipEllipse and these functions is slightly unfair.
 *
 * \since 2.00 Beta 5
 */

/** @defgroup outlinedcircle Drawing of circle outline
 * @ingroup circle
 * \brief These functions draw the outline of the circle of radius \a radius centered at (\a xcenter, \a ycenter) in one or two 240x128 video plane(s), with hard-coded drawing mode.
 *
 * The non-clipped versions are significantly faster than the clipped versions.
 *
 * \warning GrayClipFastOutlinedCircle*_R require consecutive grayscale planes (see <a href="../../extgraph.html#grayscaletilemap">the
 * root of the ExtGraph documentation</a> for more information), in order not to use too many registers, which would make the used algorithm less efficient.<br>
 * <b>NOT PROVIDING SUCH PLANES IS LIKELY TO CRASH HW1 CALCULATORS</b> (which have become VERY infrequent in 2009, but still...).
 *
 * \todo Generic FastOutlinedCircle functions with self-modifying code, that can draw more than one mode ?
 */
//@{
void FastOutlinedCircle_DRAW_R(void *plane asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void FastOutlinedCircle_ERASE_R(void *plane asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void FastOutlinedCircle_INVERT_R(void *plane asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void ClipFastOutlinedCircle_DRAW_R(void *plane asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void ClipFastOutlinedCircle_ERASE_R(void *plane asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void ClipFastOutlinedCircle_INVERT_R(void *plane asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));

void GrayClipFastOutlinedCircle_BLACK_R(void *planes asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void GrayClipFastOutlinedCircle_LGRAY_R(void *planes asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void GrayClipFastOutlinedCircle_DGRAY_R(void *planes asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void GrayClipFastOutlinedCircle_WHITE_R(void *planes asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
void GrayClipFastOutlinedCircle_INVERT_R(void *planes asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"));
//@}

/** @defgroup filledcircle Drawing of circle and interior (disk)
 * @ingroup circle
 * \brief These functions fill the circle of radius \a radius centered at (\a xcenter, \a ycenter) in one or two 240x128 video plane(s), with hard-coded drawing mode.
 *
 * \warning \ref GrayClipFastFilledCircle_R, of which the \a drawfunc parameter must be a routine compatible with GrayDrawSpan* ones,
 * requires consecutive grayscale planes (see <a href="../../extgraph.html#grayscaletilemap">the root of the ExtGraph documentation</a> for 
 * more information), in order not to use too many registers, which would make the used algorithm less efficient.<br>
 * <b>NOT PROVIDING SUCH PLANES IS LIKELY TO CRASH HW1 CALCULATORS</b> (which have become VERY infrequent in 2009, but still...).
 */
//@{
void ClipFastFilledCircle_R(void *plane asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"), void(*drawfunc)(short x1 asm("%d0"), short x2 asm("%d1"), void * addr asm("%a0")) asm("%a2"));
void GrayClipFastFilledCircle_R(void *planes asm("%a0"), short xcenter asm("%d0"), short ycenter asm("%d1"), unsigned short radius asm("%d2"), void(*drawfunc)(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0")) asm("%a2"));
//@}



// -----------------------------------------------------------------------------
/** @defgroup triangle Triangle drawing functions
 * \brief These functions fill the triangle whose endpoints are (\a x1, \a y1), (\a x2, \a y2) and (\a x3, \a y3) in one or two 240x128 video plane(s), with hard-coded drawing mode.
 *
 * The \a drawfunc parameter must be a routine compatible with DrawSpan* / GrayDrawSpan* ones.
 * \warning \ref GrayFilledTriangle_R requires consecutive grayscale planes (see <a href="../../extgraph.html#grayscaletilemap">the root of the ExtGraph
 * documentation</a> for more information), in order not to use too many registers, which would make the used algorithm less efficient.<br>
 * <b>NOT PROVIDING SUCH PLANES IS LIKELY TO CRASH HW1 CALCULATORS</b> (which have become VERY infrequent in 2009, but still...).
 *
 * Can you figure the code of the outlined triangle drawing functions ? ;-)
 * \since 2.00 Beta 5.
 * \note FilledTriangle_R and GrayFilledTriangle_R are currently the exact same routine.
 */
//@{
void FilledTriangle_R(unsigned short x1 asm("%d0"), unsigned short y1 asm("%d1"), unsigned short x2 asm("%d2"), unsigned short y2 asm("%d3"), unsigned short x3 asm("%d4"), unsigned short y3 asm("%a1"), void *plane asm("%a0"), void(*drawfunc)(short x1 asm("%d0"), short x2 asm("%d1"), void * addr asm("%a0")) asm("%a2"));
void GrayFilledTriangle_R(short x1 asm("%d0"), short y1 asm("%d1"), short x2 asm("%d2"), short y2 asm("%d3"), short x3 asm("%d4"), short y3 asm("%a1"), void *planes asm("%a0"), void(*drawfunc)(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0")) asm("%a2"));
//@}



// -----------------------------------------------------------------------------
/** @defgroup spans Special line drawing functions for FilledCircle/FilledTriangle
 * @ingroup circle triangle
 * \brief Special horizontal line drawing functions fit for filled triangle and circle functions.
 *
 * Unlike (Gray)FastDrawHLine(2B)_R, these routines are clipped. The \a addr/\a addrs pointer is 
 * interpreted as the address of the beginning of the screen row to which \a x1 and \a x2
 * relate. See the code of \ref FilledTriangle_R for an example of use.
 */
//@{
void DrawSpan_OR_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addr asm("%a0"));
void DrawSpan_XOR_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addr asm("%a0"));
void DrawSpan_REVERSE_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addr asm("%a0"));

void GrayDrawSpan_WHITE_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0"));
void GrayDrawSpan_LGRAY_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0"));
void GrayDrawSpan_DGRAY_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0"));
void GrayDrawSpan_BLACK_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0"));
void GrayDrawSpan_INVERT_R(short x1 asm("%d0"), short x2 asm("%d1"), void * addrs asm("%a0"));
//@}



// -----------------------------------------------------------------------------
//! @defgroup scaling Sprite scaling functions

/** @defgroup scaledraw Scaling+drawing of sprites
 * @ingroup scaling
 * \brief Scale a <b>square</b> sprite of various widths (depending on the name of the function) to \a sizex x \a sizey pixels, while drawing the scaled sprite at (\a x0, \a y0) in 240-pixel-wide video planes pointed to by \a dest, with hard-coded drawing mode.
 *
 * \warning No clipping is done !
 * \author Julien Richard-Foy
 * \todo Functions that draw the scaled sprite as sprite data, not to a plane.
 */ 
//@{
void ScaleSprite8_OR(const unsigned char *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite16_OR(const unsigned short *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite32_OR(const unsigned long *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite64_OR(const unsigned long long *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));

void ScaleSprite8_AND(const unsigned char *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite16_AND(const unsigned short *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite32_AND(const unsigned long *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite64_AND(const unsigned long long *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));

void ScaleSprite8_XOR(const unsigned char *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite16_XOR(const unsigned short *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite32_XOR(const unsigned long *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
void ScaleSprite64_XOR(const unsigned long long *sprite, void *dest, short x0, short y0, short sizex, short sizey) __attribute__((__stkparm__));
//@}

/** @defgroup doublespritedimensions x2 sprite scaling functions
 * @ingroup scaling
 * \brief Scaling functions optimized for x2 scale factor.
 *
 */
//@{
//! Scale by a factor of two in each direction a (\a bytewidth *8)x\a height sprite pointed to by \a src, storing the result as sprite data in are pointed to by \a dest.
void DoubleSpriteDimensionsX8_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned short bytewidth asm("%d1"), unsigned short* dest asm("%a1")) __attribute__((__regparm__(4)));
//! Scale by a factor of two in each direction the 16x16 sprite pointed to by \a src, writing the result to the area pointed to by \a dest.<br>
//! DoubleSpriteDimensions16x16_R is designed for use in file explorers to read the AMS native comments available in TIGCC 0.95+
void DoubleSpriteDimensions16x16_R(const unsigned short* src asm("%a0"), unsigned long* dest asm("%a1")) __attribute__((__regparm__(2)));
//@}



// -----------------------------------------------------------------------------
//! @defgroup planescaling Plane scaling functions
//! These functions can be used to scale down 92+/V200 graphics to 89/89T screen, or scale up 89/89T to 92+/V200 screen.
//! \note By their nature, these functions are rather slow, and they yield ugly results. You have been warned.
//! \since 2.00 beta 5
// -----------------------------------------------------------------------------
//@{
//! Scale 240-pixel-wide plane pointed to by src so that only 160 pixels out of the 240 pixels of each line of dest are used
//! \author Geoffrey Anneheim
void Scale1Plane240to160_R(const void *src asm("%a0"), void *dest asm("%a1"));
//! Scale 160 pixels of a 240-pixel-wide plane so that 240 pixels of each line of dest are used
//! \author GoldenCrystal
void Scale1Plane160to240_R(const void *src asm("%a0"), void *dest asm("%a1"));
//@}



// -----------------------------------------------------------------------------
//! @defgroup fastscreen Plane copy/modify operations
//! Fast functions for complete screen (240x128 pixels == 3840 bytes) operations.
// -----------------------------------------------------------------------------
//@{
//! Copy 240x128 plane pointed to by \a src to 240x128 plane pointed to by \a dest
//! \note \a src and \a dest must start at an even address.
//! \warning FastCopyScreen will crash your calculator if \a src points into the archive memory, for example if you try to copy a screen
//! which is stored in an archived variable. DON'T use FastCopyScreen in this case.
//! \deprecated __stkparm__ function with equivalent __regparm__ function
void FastCopyScreen(const void* src, void* dest) __attribute__((__stkparm__));
//! Copy 240x128 plane pointed to by \a src to 240x128 plane pointed to by \a dest, register-parameter-passing version
//! \note \a src and \a dest must start at an even address.
//! \warning FastCopyScreen_R will crash your calculator if \a src points into the archive memory, for example if you try to copy a screen
//! which is stored in an archived variable. DON'T use FastCopyScreen in this case.
void FastCopyScreen_R(const void* src asm("%a0"), void* dest asm("%a1")) __attribute__((__regparm__(2)));
//! Copy 160x\a height screen (bytewidth = 20) pointed to by \a src <b>to</b> the upper-left corner of 240x128 plane pointed to by \a dest.
//! \note \a src and \a dest must start at an even address.
//! \since 2.00 Beta 5
void FastCopyScreen160to240_R(unsigned short height asm("%d0"), const void* src asm("%a0"), void* dest asm("%a1")) __attribute__((__regparm__(3)));
//! Copy 160x\a height screen (bytewidth = 20) pointed to by \a src <b>near</b> the center of 240x128 plane pointed to by \a dest.
//! \note \a src and \a dest must start at an even address.
//! The upper left corner of 160x\a height data is at (32, 14+(100-\a height)/2), which makes the copy process significantly more efficient than that of \ref FastCopyScreen160to240_R.
//! \since 2.00 Beta 5
void FastCopyScreen160to240NC_R(unsigned short height asm("%d0"), const void* src asm("%a0"), void* dest asm("%a1")) __attribute__((__regparm__(3)));
//! AND 240x128 plane pointed to by \a src to 240x128 plane pointed to by \a dest.
//! \note \a src and \a dest must start at an even address.
//! \since 2.00 Beta 5
void FastANDScreen_R(const void* src asm("%a0"), void* dest asm("%a1")) __attribute__((__regparm__(2)));
//! OR 240x128 plane pointed to by \a src to 240x128 plane pointed to by \a dest.
//! \note \a src and \a dest must start at an even address.
//! \since 2.00 Beta 5
void FastORScreen_R(const void* src asm("%a0"), void* dest asm("%a1")) __attribute__((__regparm__(2)));
//! XOR 240x128 plane pointed to by \a src to 240x128 plane pointed to by \a dest.
//! \note \a src and \a dest must start at an even address.
//! \since 2.00 Beta 5
void FastXORScreen_R(const void* src asm("%a0"), void* dest asm("%a1")) __attribute__((__regparm__(2)));
//! Invert bits of 240x128 plane pointed to by \a src.
//! \note \a src must start at an even address.
//! \since 2.00 Beta 6
void FastInvertScreen_R(const void* src asm("%a0")) __attribute__((__regparm__(1)));
//@}


//! @defgroup drawlargebuffer Large buffer -> 240x128 plane drawing functions
//! @ingroup fastscreen
//@{
//! Replace the contents of 240x128 \a dest buffer with data from the rectangular area (\a offsetx, \a offsety, \a offsetx+239, \a offsety+127) of the (\a wordwidth *16) x \a height source buffer pointed to by \a big_screen.
//! \todo draw something if the result of the clipping is less than 240x128 pixels.
void FastDrawLargeBufferToScreen_R(const void * big_screen asm("%a0"), void * dest asm("%a1"), unsigned short offsetx asm("%d0"), unsigned short offsety asm("%d1"), unsigned short wordwidth asm("%d2"), unsigned short height asm("%d3")) __attribute__((__regparm__(6)));
//@}


// -----------------------------------------------------------------------------
/** @defgroup floodfill FloodFill functions
 * \brief 4-way floodfill routines
 *
 * These routines fill the interior of an area which enclosed by an arbitrary shaped figure (a circle, a polygon, etc) using a 4-way floodfill algorithm.<br>
 * Parameters \a x and \a y specify the point in the 240x128 destination plane (\a dest) from where the filling process should be started. If the pixel at (\a x, \a y) is already set the routine returns immediately.
 *
 * Parameter \a shade is a 16-bit unsigned integer which defines a 4x4 matrix of pixels used to fill the interior (for example: if shade is set to 0xFFFF the interior is filled completely black). Routines of the "noshade" kind hard-code shade to 0xFFFF to squeeze a bit more of speed.<br>
 * Parameter \a tmpplane has to be a 240x128 pixels large "scratch" buffer which is used internally. Routines of the "MF" kind don't need this additional parameter, they use malloc and free internally to allocate and release the buffer.<br>
 * \note Due to the fact that FloodFill heavily uses the common program stack for its operation, the given tmpplane buffer shouldn't be a local LCD_BUFFER variable, but it should be allocated dynamically by using malloc() like FloodFillMF does internally. 
 * \note If speed is crucial you should use FloodFill* instead of FloodFillMF*.
 * \author Zeljko Juric
 */
// -----------------------------------------------------------------------------
//@{
void FloodFill(unsigned short x, unsigned short y, unsigned short shade, void* tmpplane, void* dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void FloodFill_R(unsigned short x, unsigned short y, unsigned short shade, void* tmpplane, void* dest) __attribute__((__regparm__(5)));
void FloodFill_noshade_R (unsigned short x, unsigned short y, void* tmpplane, void* dest) __attribute__((__regparm__(4)));

void FloodFillMF(unsigned short x, unsigned short y, unsigned short shade, void* dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void FloodFillMF_R(unsigned short x, unsigned short y, unsigned short shade, void* dest) __attribute__((__regparm__(4)));
void FloodFillMF_noshade_R (unsigned short x, unsigned short y, void* dest) __attribute__((__regparm__(3)));
//@}



// -----------------------------------------------------------------------------
/** @defgroup sprite Sprite drawing functions
 * Description of the drawing modes:
 * <ul>
 *   <li><b>AND</b>, <b>OR</b>, <b>XOR</b> modes are the usual "clear pixels", "set pixels", "invert pixels" routines</li>
 *   <li><b>MASK</b> mode ANDs the background with the full-sized mask, before ORing the background with the sprite</li>
 *   <li><b>BLIT</b> mode ANDs the background with the same 1-line mask applied to all lines, before ORing the background with the sprite</li>
 *   <li><b>RPLC</b> mode replaces the background with the sprite (it is a BLIT with a hard-coded mask of 0s)</li>
 *   <li><b>Get</b> makes from the background a sprite usable by other sprite & tile functions. Much faster than the OS BitmapGet functions, but not compatible</li>
 *   <li><b>SMASK</b> (grayscale only) ANDs both planes with the same full-sized mask, before ORing each plane with a different sprite</li>
 *   <li><b>TRANB</b> (grayscale only) draws the sprite as if the black color were transparent</li>
 *   <li><b>TRAND</b> (grayscale only) draws the sprite as if the dark gray color were transparent</li>
 *   <li><b>TRANL</b> (grayscale only) draws the sprite as if the light gray color were transparent</li>
 *   <li><b>TRANW</b> (grayscale only) draws the sprite as if the white color were transparent</li>
 * </ul>
 */
// -----------------------------------------------------------------------------
/** @defgroup ncspsprite Non-clipped, single-plane sprite drawing functions
 * @ingroup sprite
 * \brief Non-clipped single-plane sprite functions.
 *
 * AND, OR & XOR routines are equivalent to (but faster than) the Sprite8/16/32 functions of TIGCCLIB.<br>
 * See the description of the drawing modes in \ref sprite.
 */
// -----------------------------------------------------------------------------
//@{
void Sprite8_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite8_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite8_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, const unsigned char maskval, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite8_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char maskval asm("%d3"), void *dest asm("%a0"));
void Sprite8Get(unsigned short x, unsigned short y, unsigned short height, const void* src, unsigned char* dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite8Get_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src asm("%a0"), unsigned char* dest asm("%a1")) __attribute__((__regparm__));
void Sprite8_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, const unsigned char *mask, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite8_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *mask, void *dest asm("%a0")) __attribute__((__stkparm__));
void Sprite8_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite8_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite8_RPLC_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite8_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite8_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));


void Sprite16_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned short *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite16_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite16_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned short *sprt, const unsigned short maskval, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite16_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), const unsigned short maskval asm("%d3"), void *dest asm("%a0"));
void Sprite16Get(unsigned short x, unsigned short y, unsigned short height, const void* src, unsigned short* dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite16Get_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src asm("%a0"), unsigned short* dest asm("%a1")) __attribute__((__regparm__));
void Sprite16_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned short *sprt, const unsigned short *mask, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite16_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), const unsigned short *mask, void *dest asm("%a0")) __attribute__((__stkparm__));
void Sprite16_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned short *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite16_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite16_RPLC_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite16_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned short *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite16_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));


void Sprite32_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned long *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite32_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite32_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned long *sprt, const unsigned long maskval, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite32_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), const unsigned long maskval asm("%d3"), void *dest asm("%a0"));
void Sprite32Get(unsigned short x, unsigned short y, unsigned short height, const void* src, unsigned long* dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite32Get_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src asm("%a0"), unsigned long* dest asm("%a1")) __attribute__((__regparm__));
void Sprite32_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned long *sprt, const unsigned long *mask, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite32_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), const unsigned long *mask, void *dest asm("%a0")) __attribute__((__stkparm__));
void Sprite32_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned long *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite32_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite32_RPLC_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void Sprite32_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned long *sprt, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void Sprite32_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));


void SpriteX8_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, unsigned short bytewidth, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__regparm__));
void SpriteX8_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, const unsigned char *maskval, unsigned short bytewidth, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *maskval, unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__stkparm__));
void SpriteX8Get(unsigned short x, unsigned short y, unsigned short height, const void* src, unsigned char* dest, unsigned short bytewidth) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8Get_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void *src asm("%a1"), unsigned char *dest asm("%a0"), unsigned short bytewidth asm("%d3")) __attribute__((__regparm__));
void SpriteX8_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, const unsigned char *mask, unsigned short bytewidth, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *mask, unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__stkparm__));
void SpriteX8_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, unsigned short bytewidth, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__regparm__));
void SpriteX8_RPLC_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__regparm__));
void SpriteX8_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned char *sprt, unsigned short bytewidth, void *dest) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__regparm__));
//@}


//@{
void SlowerSpriteX8_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__regparm__));
void SlowerSpriteX8_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *maskval, unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__stkparm__));
void SlowerSpriteX8Get_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void *src asm("%a1"), unsigned char *dest asm("%a0"), unsigned short bytewidth asm("%d3")) __attribute__((__regparm__));
void SlowerSpriteX8_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *mask, unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__stkparm__));
void SlowerSpriteX8_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__regparm__));
void SlowerSpriteX8_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), unsigned short bytewidth asm("%d3"), void *dest asm("%a0")) __attribute__((__regparm__));
//@}



// -----------------------------------------------------------------------------
/** @defgroup cspsprite Clipped, single-plane sprite drawing functions
 * @ingroup sprite
 * \brief Clipped single-plane sprite functions.
 *
 * See the description of the drawing modes in \ref sprite.
 * \note Clear the destination buffer before using ClipSprite*Get_R.
 */
// -----------------------------------------------------------------------------
//@{
void ClipSprite8_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite8_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char maskval asm("%d3"), void *dest asm("%a0"));
void ClipSprite8_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *mask, void *dest asm("%a0")) __attribute__((__stkparm__));
void ClipSprite8_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite8_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite8_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite8Get_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), void *dest asm("%a0"), unsigned char *sprt asm("%a1"));


void ClipSprite16_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite16_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), const unsigned short maskval asm("%d3"), void *dest asm("%a0"));
void ClipSprite16_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), const unsigned short *mask, void *dest asm("%a0")) __attribute__((__stkparm__));
void ClipSprite16_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite16_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite16_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite16Get_R(short x asm("%d0"),short y asm("%d1"),unsigned short height asm("%d2"), void *dest asm("%a0"), unsigned short *sprt asm("%a1"));


void ClipSprite32_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite32_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), const unsigned long maskval asm("%d3"), void *dest asm("%a0"));
void ClipSprite32_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), const unsigned long *mask, void *dest asm("%a0")) __attribute__((__stkparm__));
void ClipSprite32_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite32_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite32_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt asm("%a1"), void *dest asm("%a0"));
void ClipSprite32Get_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), void *dest asm("%a0"), unsigned long *sprt asm("%a1"));


void ClipSpriteX8_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), unsigned short bytewidth asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void ClipSpriteX8_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), unsigned short bytewidth asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *maskval asm("%a2"), void *dest asm("%a0")) __attribute__((__regparm__));
void ClipSpriteX8_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), unsigned short bytewidth asm("%d2"), const unsigned char *sprt asm("%a1"), const unsigned char *mask asm("%a2"), void *dest asm("%a0")) __attribute__((__regparm__));
void ClipSpriteX8_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), unsigned short bytewidth asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void ClipSpriteX8_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), unsigned short bytewidth asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void ClipSpriteX8_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), unsigned short bytewidth asm("%d2"), const unsigned char *sprt asm("%a1"), void *dest asm("%a0")) __attribute__((__regparm__));
void ClipSpriteX8Get_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), unsigned short bytewidth asm("%d2"), void *dest asm("%a0"), unsigned char *sprt asm("%a1")) __attribute__((__regparm__));
//@}



// -----------------------------------------------------------------------------
/** @defgroup nctpsprite Non-clipped, two-plane sprite drawing functions
 * @ingroup sprite
 * \brief Non-clipped two-plane sprite functions.
 *
 * See the description of the drawing modes in \ref sprite.
 */
// -----------------------------------------------------------------------------
//@{
void GraySprite8_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite8_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, const unsigned char maskval, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite8_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, const unsigned char* mask1, const unsigned char* mask2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite8_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char *mask0, const unsigned char *mask1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite8_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_RPLC_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_SMASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char *mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_TRANB_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_TRANW_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite8_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite8_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


void GraySprite16_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned short* sprite1, const unsigned short* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite16_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned short* sprite1, const unsigned short* sprite2, const unsigned short maskval, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite16_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned short* sprite1, const unsigned short* sprite2, const unsigned short* mask1, const unsigned short* mask2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite16_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short *mask0, const unsigned short *mask1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned short* sprite1, const unsigned short* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite16_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_RPLC_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_SMASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short *mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_TRANB_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_TRANW_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite16_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned short* sprite1, const unsigned short* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite16_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


void GraySprite32_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned long* sprite1, const unsigned long* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite32_AND_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned long* sprite1, const unsigned long* sprite2, const unsigned long maskval, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite32_BLIT_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned long* sprite1, const unsigned long* sprite2, const unsigned long* mask1, const unsigned long* mask2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite32_MASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long *mask0, const unsigned long *mask1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned long* sprite1, const unsigned long* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite32_OR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_RPLC_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_SMASK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long *mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_TRANB_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_TRANW_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySprite32_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned long* sprite1, const unsigned long* sprite2, void* dest1, void* dest2) __attribute__((__stkparm__)); ///< \deprecated __stkparm__ function with equivalent __regparm__ function
void GraySprite32_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


// Not currently rewritten, transparency functions and __regparm__ versions
// were not written either...
void GraySpriteX8_AND(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, unsigned short bytewidth, void* dest1, void* dest2) __attribute__((__stkparm__));
void GraySpriteX8_BLIT(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, const unsigned char* maskval, unsigned short bytewidth, void* dest1, void* dest2) __attribute__((__stkparm__));
void GraySpriteX8_MASK(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, const unsigned char* mask1, const unsigned char* mask2, unsigned short bytewidth, void* dest1, void* dest2) __attribute__((__stkparm__));
void GraySpriteX8_OR(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, unsigned short bytewidth, void* dest1, void* dest2) __attribute__((__stkparm__));
void GraySpriteX8_XOR(unsigned short x, unsigned short y, unsigned short height, const unsigned char* sprite1, const unsigned char* sprite2, unsigned short bytewidth, void* dest1, void* dest2) __attribute__((__stkparm__));
//@}


// -----------------------------------------------------------------------------
/** @defgroup ctpsprite Clipped, two-plane sprite drawing functions
 * @ingroup sprite
 * \brief Clipped two-plane sprite functions.
 *
 * See the description of the drawing modes in \ref sprite.
 */
// -----------------------------------------------------------------------------
//@{
void GrayClipSprite8_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char *mask0, const unsigned char *mask1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_SMASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char *mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_TRANB_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_TRANW_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite8_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


void GrayClipSprite16_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short *mask0, const unsigned short *mask1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_SMASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short *mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//! Combined SMASK and BLIT operation.
void GrayClipSprite16_SMASKBLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), unsigned short *sprt0, unsigned short *sprt1, unsigned short *mask, const unsigned short maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_TRANB_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_TRANW_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//! Draw sprite upside down.
void UpsideDownGrayClipSprite16_MASK_R(register short x asm("%d0"),register short y asm("%d1"),register unsigned short height asm("%d2"), unsigned short *sprt0, unsigned short *sprt1, unsigned short *mask0, unsigned short *mask1,register void *dest0 asm("%a0"),register void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite16_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


void GrayClipSprite32_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long *mask0, const unsigned long *mask1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_SMASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long *mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_TRANB_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_TRANW_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipSprite32_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//@}


// MastaZog between others asked for GrayClipSpriteX8_X_R functions...



// -----------------------------------------------------------------------------
/** @defgroup ctpisprite Clipped, two-plane sprite drawing functions, interlaced sprite format
 * @ingroup sprite
 * \brief Clipped two-plane interlaced sprite functions.
 *
 * See the description of the drawing modes in \ref sprite.
 *
 * The interlaced sprite format is as follows:
 * <ul><li>all functions except MASK: L/D or D/L (depending on the convention you use
 *   for dest0 and dest1, usually LIGHT_PLANE and DARK_PLANE respectively, but
 *   you can do otherwise), without mask.</li>
 * <li>MASK: M/L/D or M/D/L (depending on the convention you use for dest0 and
 *   dest1, usually LIGHT_PLANE and DARK_PLANE respectively, but you can do
 *   otherwise), mask is applied to both planes.</li></ul>
 *
 * With some amount of work, depending on your knowledge of assembly, you
 * can make other TRAND/L functions from GrayClipISprite16_TRAND/L_R.
 *
 * As you can guess, GrayClipISpriteX16_MASK_R draws an interlaced masked
 * sprite whose width is multiple of 16 pixels, with the "interlacement size"
 * (size of a M/L/D chunk) being 3*2 bytes.
 */
//@{
void GrayClipISprite8_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite8_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, const unsigned char maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite8_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite8_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite8_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite8_TRANB_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite8_TRANW_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite8_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


void GrayClipISprite16_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, const unsigned short maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_TRANB_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_TRAND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_TRANL_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_TRANW_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite16_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


void GrayClipISprite32_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite32_BLIT_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, const unsigned long maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite32_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite32_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite32_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite32_TRANB_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite32_TRANW_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISprite32_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d2"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));


void GrayClipISpriteX16_AND_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), const unsigned short *sprt, unsigned short wordwidth asm("%d2"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISpriteX16_MASK_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), const unsigned short *sprt, unsigned short wordwidth asm("%d2"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISpriteX16_OR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), const unsigned short *sprt, unsigned short wordwidth asm("%d2"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISpriteX16_RPLC_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), const unsigned short *sprt, unsigned short wordwidth asm("%d2"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayClipISpriteX16_XOR_R(short x asm("%d0"), short y asm("%d1"), unsigned short height asm("%d3"), const unsigned short *sprt, unsigned short wordwidth asm("%d2"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//@}



// -----------------------------------------------------------------------------
/** @defgroup nctpssprite Non-clipped, two-plane, single sprite drawing functions
 * @ingroup sprite
 * \brief These functions draw a single sprite to two planes
 *
 * \author David Randall, Lionel Debroux.
 * \todo 16, 32, X8 routines ?
 */
// -----------------------------------------------------------------------------
//@{
void GraySingleSprite8_BLACK_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//! Parameter \a color is an element of enum \ref GrayColors.
void GraySingleSprite8_COLOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt, short color, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySingleSprite8_DGRAY_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySingleSprite8_LGRAY_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySingleSprite8_WHITE_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GraySingleSprite8_XOR_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const unsigned char *sprt, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//@}



// -----------------------------------------------------------------------------
/** @defgroup tile Tile (aligned sprite) drawing functions
 * \brief Aligned, square sprite functions.
 *
 * The drawing modes are the same as those of the \ref sprite.
 *
 * \note None of the tile functions is clipped.
 * \note These 'Tile' functions have nothing to do with the tilemap engine.
 * \note Supporting variable height tiles is easy:
 * <ul><li>copy the functions in your project;</li>
 * <li>add the \a height parameter in d2;</li>
 * <li>modify the functions (everything outside the loops related to d2).</li></ul>
 * <i>Warning</i>, most loops are unrolled: if you don't want to modify the routine more thoroughly, the height will often have to be
 * multiple of 2 or even 4.
 */
// -----------------------------------------------------------------------------
/** @defgroup sptile Single-plane tile drawing functions
 * @ingroup tile
 * \brief Non-clipped single plane tile (aligned sprite) functions, non-interlaced sprite format
 *
 * These functions draw square tiles of various widths (depending on the function's name) pointed to by \a sprite at (8*\a col,y) in \a plane.<br>
 * These functions have nothing to do with the tilemap engine.
 * \warning Passing an odd value for \a col to 16x16 and 32x32 routines triggers a CRASH !
 */
// -----------------------------------------------------------------------------
//@{
void Tile8x8_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite asm("%a1"), void *plane asm("%a0"));
void Tile8x8_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite asm("%a1"), const unsigned char maskval asm("%d3"), void *plane asm("%a0"));
void Tile8x8Get_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const void *src asm("%a0"), unsigned char *dest asm("%a1"));
void Tile8x8_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite asm("%a1"), const unsigned char* mask, void *plane asm("%a0")) __attribute__((__stkparm__));
void Tile8x8_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite asm("%a1"), void *plane asm("%a0"));
void Tile8x8_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite asm("%a1"), void *plane asm("%a0"));
void Tile8x8_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite asm("%a1"), void *plane asm("%a0"));

void Tile16x16_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite asm("%a1"), void *plane asm("%a0"));
void Tile16x16_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite asm("%a1"), const unsigned short maskval asm("%d3"), void *plane asm("%a0"));
void Tile16x16Get_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const void *src asm("%a0"), unsigned short *dest asm("%a1"));
void Tile16x16_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite asm("%a1"), const unsigned short* mask, void *plane asm("%a0")) __attribute__((__stkparm__));
void Tile16x16_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite asm("%a1"), void *plane asm("%a0"));
void Tile16x16_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite asm("%a1"), void *plane asm("%a0"));
void Tile16x16_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite asm("%a1"), void *plane asm("%a0"));

void Tile32x32_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite asm("%a1"), void *plane asm("%a0"));
void Tile32x32_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite asm("%a1"), const unsigned long maskval asm("%d3"), void *plane asm("%a0"));
void Tile32x32Get_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const void *src asm("%a0"), unsigned long *dest asm("%a1"));
void Tile32x32_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite asm("%a1"), const unsigned long* mask, void *plane asm("%a0")) __attribute__((__stkparm__));
void Tile32x32_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite asm("%a1"), void *plane asm("%a0"));
void Tile32x32_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite asm("%a1"), void *plane asm("%a0"));
void Tile32x32_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite asm("%a1"), void *plane asm("%a0"));
//@}



// -----------------------------------------------------------------------------
/** @defgroup tptile Two-plane tile drawing functions
 * @ingroup tile
 * \brief Non-clipped two-plane tile (aligned sprite) functions, non-interlaced sprite format
 *
 * These functions draw square tiles of various widths (depending on the function's name) pointed to by \a sprt0 and \a sprt1 at (8*\a col,y) in planes \a dest0 and \a dest1.<br>
 * \warning Passing an odd value for \a col to 16x16 and 32x32 routines triggers a CRASH !
 */
// -----------------------------------------------------------------------------
//@{
void GrayTile8x8_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char* mask1, const unsigned char* mask2, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_SMASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, const unsigned char* mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_TRANB_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_TRANW_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile8x8_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprt0, const unsigned char *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));

void GrayTile16x16_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, const unsigned short* mask1, const unsigned short* mask2, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_SMASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, unsigned short* mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_TRANB_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_TRANW_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile16x16_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprt0, const unsigned short *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));

void GrayTile32x32_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, const unsigned long* mask1, const unsigned long* mask2, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_SMASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, unsigned long* mask, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_TRANB_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_TRANW_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayTile32x32_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprt0, const unsigned long *sprt1, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//@}



// -----------------------------------------------------------------------------
/** @defgroup tpitile Two-plane tile drawing functions, interlaced sprite format
 * @ingroup tile
 * \brief Non-clipped two-plane tile (aligned sprite) functions, interlaced sprite format
 *
 * These functions draw square tiles of various widths (depending on the function's name) pointed to by \a sprite at (8*\a col, \a y) in planes \a dest0 and \a dest1.<br>
 * The interlaced sprite format is the same as that of the \ref ctpisprite
 * \warning Passing an odd value for \a col to 16x16 and 32x32 routines triggers a CRASH !
 */
// -----------------------------------------------------------------------------
//@{
void GrayITile8x8_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile8x8_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, const unsigned char maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile8x8_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile8x8_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile8x8_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile8x8_TRANB_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile8x8_TRANW_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile8x8_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned char *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));

void GrayITile16x16_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile16x16_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, const unsigned short maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile16x16_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile16x16_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile16x16_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile16x16_TRANB_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile16x16_TRANW_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile16x16_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned short *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));

void GrayITile32x32_AND_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile32x32_BLIT_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, const unsigned long maskval asm("%d3"), void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile32x32_MASK_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile32x32_OR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile32x32_RPLC_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile32x32_TRANB_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile32x32_TRANW_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
void GrayITile32x32_XOR_R(unsigned short col asm("%d0"), unsigned short y asm("%d1"), const unsigned long *sprite, void *dest0 asm("%a0"), void *dest1 asm("%a1")) __attribute__((__stkparm__));
//@}



// -----------------------------------------------------------------------------
/** @defgroup transeff Transition effects
 * \brief Various transition effects
 * 
 * I find it hard to describe what some of these routines do, since the effect is very visual.
 * I suggest reading and executing demo18 (which also provides a hint about the \a wait parameter) :-)
 * 
 * \todo many missing effects detailed in <a href="../../../todo%20extgraph.txt">the general ExtGraph todo list</a>
 */
// -----------------------------------------------------------------------------
//@{
void FadeOutToBlack_CWS1_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_CWS1_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));

void FadeOutToBlack_CCWS1_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_CCWS1_R(void* lightplane asm("%a0"), void* darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));


void FadeOutToBlack_LR_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_LR_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToBlack_LR18_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_LR18_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToBlack_LR28_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_LR28_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));

void FadeOutToBlack_RL_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_RL_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToBlack_RL18_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_RL18_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToBlack_RL28_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_RL28_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));


void FadeOutToBlack_TB_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_TB_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));

void FadeOutToBlack_BT_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));
void FadeOutToWhite_BT_R(void *lightplane asm("%a0"), void *darkplane asm("%a1"), unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), short wait asm("%d2"));

//! Make (\a wordwidth *16)x\a height planes starting at \a dest0 and \a dest1 lighter.
void GrayIShadowPlanesX16_R(void *dest0 asm("%a0"), void *dest1 asm("%a1"), unsigned short height asm("%d0"), unsigned short wordwidth asm("%d1")) __attribute__((__regparm__(4)));
//! Make (\a wordwidth *16)x\a height planes starting at \a dest0 and \a dest1 darker.
void GrayShadowPlanesX16_R(void *dest0 asm("%a0"), void *dest1 asm("%a1"), unsigned short height asm("%d0"), unsigned short wordwidth asm("%d1")) __attribute__((__regparm__(4)));
//! Make 240x\a height planes starting at \a dest0 and \a dest1 lighter.
void GrayIShadowPlanes240_R(void *dest0 asm("%a0"), void *dest1 asm("%a1"), unsigned short height asm("%d0")) __attribute__((__regparm__(3)));
//! Make 240x\a height planes starting at \a dest0 and \a dest1 darker.
void GrayShadowPlanes240_R(void *dest0 asm("%a0"), void *dest1 asm("%a1"), unsigned short height asm("%d0")) __attribute__((__regparm__(3)));
//! Make 160x\a height planes (bytewidth=20) starting at \a dest0 and \a dest1 lighter.
void GrayIShadowPlanes160_R(void *dest0 asm("%a0"), void *dest1 asm("%a1"), unsigned short height asm("%d0")) __attribute__((__regparm__(3)));
//! Make 160x\a height planes (bytewidth=20) starting at \a dest0 and \a dest1 darker.
void GrayShadowPlanes160_R(void *dest0 asm("%a0"), void *dest1 asm("%a1"), unsigned short height asm("%d0")) __attribute__((__regparm__(3)));
//! Combination of \ref GrayIShadowPlanes240_R and \ref FastCopyScreen_R : make 240x\a height planes starting at \a dest0 and \a dest1 lighter and store the result in \a dest0 and \a dest1.
void GrayIShadowPlanesTo_R(const void *src0 asm("%a0"), const void *src1 asm("%a1"), void *dest0, void *dest1) __attribute__((__stkparm__));
//! Combination of \ref GrayShadowPlanes240_R and \ref FastCopyScreen_R : make 240x\a height planes starting at \a dest0 and \a dest1 darker and store the result in \a dest0 and \a dest1.
void GrayShadowPlanesTo_R(const void *src0 asm("%a0"), const void *src1 asm("%a1"), void *dest0, void *dest1) __attribute__((__stkparm__));

//! Fill 240x128 plane pointed to by \a plane with 1s.
void FastFillScreen_R(void *plane asm("%a0"));
//! Fill 240x128 plane pointed to by \a plane with 0s.
void FastClearScreen_R(void *plane asm("%a0"));
/** \brief Fills (\a len - \a len%4) bytes of plane with LFSR-generated garbage, initialized with \a seed, starting at address \a plane.
 *
 * This is less random but two orders of magnitude faster than other pseudo-random generators like twice the number of writes of <code>random(RAND_MAX)+random(RAND_MAX)</code>.
 */
void FillScreenWithGarbage_R(unsigned long seed asm("%d0"), unsigned short len asm("%d1"), void *plane asm("%a0"));
//@}



// -----------------------------------------------------------------------------
/** @defgroup spriteshadow Sprite shadow (darker) / inverse shadow (lighter) creation functions
 * \brief Helper functions to create shadows (darker) and inverse shadows (lighter) from sprites
 * 
 * 'Shadow' functions generate darker (white -> lightgray, lightgray -> darkgray, darkgray -> black, black -> black) sprites from sprites
 * of (8|16|32|\a bytewidth *8)x\a height pixels pointed to by \a src0 and \a src1, both ANDing them with \a mask,
 * and storing the result to \a dest0 and \a dest1.<br>
 * Likewise for 'IShadow' functions, except that they generate lighter (white -> white, lightgray -> white, darkgray -> lightgray, black -> darkgray)
 * sprites.
 *
 * The generated sprite shadows can be drawn with OR or MASK/SMASK functions. This takes a slightly greater amount of memory,
 * but is significantly faster at run-time (past the shadow generation phase) than using the (I)SHADOW functions introduced
 * in 2.00 Beta 3 and removed in 2.00 Beta 4 (because they were rather slow and increased maintenance).
 *
 * The interlaced sprite format of CreateISprite* routines is the same as that of the masked format as described in \ref ctpisprite . Both the
 * source and destination sprites have to be drawn with GrayClipISprite(8|16|32)_MASK routine.
 *
 * \since 2.00 Beta 4
 */
// -----------------------------------------------------------------------------
//@{
void CreateSpriteShadow8_R(unsigned short height asm("%d0"), const unsigned char* src0 asm("%a0"), const unsigned char* src1 asm("%a1"), const unsigned char* mask, unsigned char* dest0, unsigned char* dest1) __attribute__((__stkparm__));
void CreateSpriteShadow16_R(unsigned short height asm("%d0"), const unsigned short* src0 asm("%a0"), const unsigned short* src1 asm("%a1"), const unsigned short* mask, unsigned short* dest0, unsigned short* dest1) __attribute__((__stkparm__));
void CreateSpriteShadow32_R(unsigned short height asm("%d0"), const unsigned long* src0 asm("%a0"), const unsigned long* src1 asm("%a1"), const unsigned long* mask, unsigned long* dest0, unsigned long* dest1) __attribute__((__stkparm__));
void CreateSpriteShadowX8_R(unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), const unsigned char* src0 asm("%a0"), const unsigned char* src1 asm("%a1"), const unsigned char* mask, unsigned char* dest0, unsigned char* dest1) __attribute__((__stkparm__));

void CreateSpriteIShadow8_R(unsigned short height asm("%d0"), const unsigned char* src0 asm("%a0"), const unsigned char* src1 asm("%a1"), const unsigned char* mask, unsigned char* dest0, unsigned char* dest1) __attribute__((__stkparm__));
void CreateSpriteIShadow16_R(unsigned short height asm("%d0"), const unsigned short* src0 asm("%a0"), const unsigned short* src1 asm("%a1"), const unsigned short* mask, unsigned short* dest0, unsigned short* dest1) __attribute__((__stkparm__));
void CreateSpriteIShadow32_R(unsigned short height asm("%d0"), const unsigned long* src0 asm("%a0"), const unsigned long* src1 asm("%a1"), const unsigned long* mask, unsigned long* dest0, unsigned long* dest1) __attribute__((__stkparm__));
void CreateSpriteIShadowX8_R(unsigned short height asm("%d0"), unsigned short bytewidth asm("%d1"), const unsigned char* src0 asm("%a0"), const unsigned char* src1 asm("%a1"), const unsigned char* mask, unsigned char* dest0, unsigned char* dest1) __attribute__((__stkparm__));

void CreateISpriteIShadow8_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned char* dest asm("%a1"));
void CreateISpriteIShadow16_R(unsigned short height asm("%d0"), const unsigned short* src asm("%a0"), unsigned short* dest asm("%a1"));
void CreateISpriteIShadow32_R(unsigned short height asm("%d0"), const unsigned long* src asm("%a0"), unsigned long* dest asm("%a1"));

void CreateISpriteShadow8_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned char* dest asm("%a1"));
void CreateISpriteShadow16_R(unsigned short height asm("%d0"), const unsigned short* src asm("%a0"), unsigned short* dest asm("%a1"));
void CreateISpriteShadow32_R(unsigned short height asm("%d0"), const unsigned long* src asm("%a0"), unsigned long* dest asm("%a1"));
//@}



// -----------------------------------------------------------------------------
/** @defgroup fastsrb Fast background save&restore functions
 * \brief Special fast SpriteGet/RPLC functions and macros
 *
 * These routines save or restore a (8|16|32|\a bytewidth *8)x\a height pixel sprite from/to one or two 240-pixel-wide video plane(s).<br>
 * These routines are designed for programs where redrawing everything every frame is detrimental to speed.
 * See demo22 and <a href="../../ExtGraph/comparison.html">this page of the documentation</a> for hints about such situations.
 *
 * \warning
 * <ul><li>(Clip)FastGetBkgrnd8/16_R require dest being at least 4*\a height+6 bytes long (see \ref FBKGRND8_BUFSIZE and \ref FBKGRND16_BUFSIZE macros)</li>
 * <li>(Clip)FastGetBkgrnd32_R require dest being at least 6*\a height+6 bytes long (see \ref FBKGRND32_BUFSIZE macro)</li>
 * <li>Gray(Clip)FastGetBkgrnd8/16_R require dest being at least 8*\a height+6 bytes long (see \ref FGBKGRND8_BUFSIZE and \ref FGBKGRND16_BUFSIZE macros)</li>
 * <li>Gray(Glip)FastGetBkgrnd32_R require dest being at least 12*\a height+6 bytes long (see \ref FGBKGRND32_BUFSIZE macro)</li></ul>
 *
 * Special thanks go to Julien Richard-Foy: he needed such functions for one
 * of his own projects. Mine were not exactly what he was looking for, but
 * his idea of storing the offset instead of recomputing everything from the
 * parameters is great. I went further, storing:
 * <ul><li>the x coordinate (not necessarily ANDed with 15)</li>
 * <li>the offset from start of plane</li>
 * <li>the height</li></ul>
 * Hence the +6 bytes.
 */
// -----------------------------------------------------------------------------
//@{
void FastGetBkgrnd8_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src asm("%a0"), unsigned short* dest asm("%a1"));
void FastGetBkgrnd16_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src asm("%a0"), unsigned short* dest asm("%a1"));
void FastGetBkgrnd32_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src asm("%a0"), unsigned short* dest asm("%a1"));
void FastPutBkgrnd8_R(const unsigned short *sprt asm("%a1"), void *dest asm("%a0"));
void FastPutBkgrnd16_R(const unsigned short *sprt asm("%a1"), void *dest asm("%a0"));
void FastPutBkgrnd32_R(const unsigned short *sprt asm("%a1"), void *dest asm("%a0"));

void GrayFastGetBkgrnd8_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src1 asm("%a0"), const void* src2 asm("%a1"), unsigned short* dest asm("%a2"));
void GrayFastGetBkgrnd16_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src1 asm("%a0"), const void* src2 asm("%a1"), unsigned short* dest asm("%a2"));
void GrayFastGetBkgrnd32_R(unsigned short x asm("%d0"), unsigned short y asm("%d1"), unsigned short height asm("%d2"), const void* src1 asm("%a0"), const void* src2 asm("%a1"), unsigned short* dest asm("%a2"));
void GrayFastPutBkgrnd8_R(const unsigned short *sprt asm("%a2"), void *dest1 asm("%a0"), void *dest2 asm("%a1"));
void GrayFastPutBkgrnd16_R(const unsigned short *sprt asm("%a2"), void *dest1 asm("%a0"), void *dest2 asm("%a1"));
void GrayFastPutBkgrnd32_R(const unsigned short *sprt asm("%a2"), void *dest1 asm("%a0"), void *dest2 asm("%a1"));

//! Size of buffer for a \a h -pixel-high sprite retrieved by \ref FastGetBkgrnd8_R
//! \since 2.00 Beta 6
#define FBKGRND8_BUFSIZE(h)   (4*(h)+6)
//! Size of buffer for a \a h -pixel-high sprite retrieved by \ref FastGetBkgrnd16_R
//! \since 2.00 Beta 6
#define FBKGRND16_BUFSIZE(h)  (4*(h)+6)
//! Size of buffer for a \a h -pixel-high sprite retrieved by \ref FastGetBkgrnd32_R
//! \since 2.00 Beta 6
#define FBKGRND32_BUFSIZE(h)  (6*(h)+6)
//! Size of buffer for a \a h -pixel-high sprite retrieved by \ref GrayFastGetBkgrnd8_R
//! \since 2.00 Beta 6
#define FGBKGRND8_BUFSIZE(h)  (8*(h)+6)
//! Size of buffer for a \a h -pixel-high sprite retrieved by \ref GrayFastGetBkgrnd16_R
//! \since 2.00 Beta 6
#define FGBKGRND16_BUFSIZE(h) (8*(h)+6)
//! Size of buffer for a \a h -pixel-high sprite retrieved by \ref GrayFastGetBkgrnd32_R
//! \since 2.00 Beta 6
#define FGBKGRND32_BUFSIZE(h) (12*(h)+6)
//@}



// -----------------------------------------------------------------------------
// Tilemap Engine.
// -----------------------------------------------------------------------------
// See tilemap.h.
// NOTE1: the API was somewhat changed by Julien between the versions. New
// functions were added, but the most useless ones were removed, because they
// were a maintenance burden. I did the same later for (I)SHADOW(2) functions
// in extgraph.h and extgraph.a. Beta 4 and later reflect the changes in the
// tilemap engine.
// NOTE2: ExtGraph 2.00 Beta 5 is still incompatible with the official TIGCCLIB
// grayscale doublebuffering support, because the TIGCCLIB grayscale support
// does not allocate both planes consecutively on HW1), _BUT_ there is a fork,
// which is one of the two ways to use this tilemap engine with doublebuffering.
// Read the root of the ExtGraph documentation for more information.



// -----------------------------------------------------------------------------
// Brute-force preshifted sprite functions and macros.
// -----------------------------------------------------------------------------
// See preshift.h.



// -----------------------------------------------------------------------------
/** @defgroup miscspritex8 Miscellanous sprite functions
 * \brief Miscellanous SpriteX8(X8) functions (mirror, rotate).
 */
// -----------------------------------------------------------------------------
//! @defgroup spritex8mirror Sprite mirror and simple rotation functions
//! @ingroup miscspritex8
//! \note All sprite widths must be multiple of 8; the "X8X8" routines also expect the height to be multiple of 8.
//@{
//! Make a horizontal mirror (\a bytewidth *8)x\a height sprite pointed to by \a src and store the result as sprite data in the area pointed to by \a dest.
//! \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8_MIRROR_H(unsigned short height, const unsigned char* src, unsigned short bytewidth, unsigned char* dest) __attribute__((__stkparm__));
//! Register-parameter-passing version of \ref SpriteX8_MIRROR_H
void SpriteX8_MIRROR_H_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned short bytewidth asm("%d1"), unsigned char* dest asm("%a1")) __attribute__((__regparm__(4)));

//! Make a vertical mirror (\a bytewidth *8)x\a height sprite pointed to by \a src and store the result as sprite data in the area pointed to by \a dest.
//! \deprecated __stkparm__ function with equivalent __regparm__ function
void SpriteX8_MIRROR_V(unsigned short height, const unsigned char* src, unsigned short bytewidth, unsigned char* dest) __attribute__((__stkparm__));
//! Register-parameter-passing version of \ref SpriteX8_MIRROR_V
void SpriteX8_MIRROR_V_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a1"), unsigned short bytewidth asm("%d2"), unsigned char* dest asm("%a0")) __attribute__((__regparm__(4)));

//! Combination of \ref SpriteX8_MIRROR_H_R and \ref SpriteX8_MIRROR_V_R
void SpriteX8_MIRROR_HV_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned short bytewidth asm("%d1"), unsigned char* dest asm("%a1")) __attribute__((__regparm__(4)));

//! Rotate 90 \htmlonly &deg; \endhtmlonly right (\a bytewidth *8)x\a height sprite pointed to by \a src and store the result as sprite data in the area pointed to by \a dest.
//! \note The source sprite is not required to be in a writable area, but it's required to have <b>both dimensions multiple of 8</b>.
void SpriteX8X8_ROTATE_RIGHT_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned short bytewidth asm("%d1"), unsigned char* dest asm("%a1")) __attribute__((__regparm__(4)));
//! Rotate 90 \htmlonly &deg; \endhtmlonly left (\a bytewidth *8)x\a height sprite pointed to by \a src and store the result as sprite data in the area pointed to by \a dest.
//! \note The source sprite is not required to be in a writable area, but it's required to have <b>both dimensions multiple of 8</b>.
void SpriteX8X8_ROTATE_LEFT_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned short bytewidth asm("%d1"), unsigned char* dest asm("%a1")) __attribute__((__regparm__(4)));
//! Combination of \ref SpriteX8X8_ROTATE_RIGHT_R and \ref SpriteX8_MIRROR_H_R
void SpriteX8X8_RR_MH_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned short bytewidth asm("%d1"), unsigned char* dest asm("%a1")) __attribute__((__regparm__(4)));
//! Combination of \ref SpriteX8X8_ROTATE_LEFT_R and \ref SpriteX8_MIRROR_H_R
void SpriteX8X8_RL_MH_R(unsigned short height asm("%d0"), const unsigned char* src asm("%a0"), unsigned short bytewidth asm("%d1"), unsigned char* dest asm("%a1")) __attribute__((__regparm__(4)));

//! Specialized \ref FastSpriteX8_MIRROR_H_R for 8-pixel-wide sprites
//! \author Geoffrey Anneheim, Lionel Debroux
void FastSprite8_MIRROR_H_R(unsigned short height asm("%d2"), unsigned char *sprt asm("%a0"), unsigned char *dest asm("%a1"));
//! Specialized \ref FastSpriteX8_MIRROR_H_R for 16-pixel-wide sprites
//! \author Geoffrey Anneheim, Lionel Debroux
void FastSprite16_MIRROR_H_R(unsigned short height asm("%d2"), unsigned short *sprt asm("%a0"), unsigned short *dest asm("%a1"));
//! Specialized \ref FastSpriteX8_MIRROR_H_R for 32-pixel-wide sprites
//! \author Geoffrey Anneheim, Lionel Debroux
void FastSprite32_MIRROR_H_R(unsigned short height asm("%d2"), unsigned long *sprt asm("%a0"), unsigned long *dest asm("%a1"));
//! Faster (but significantly larger) than SpriteX8_MIRROR_H_R because it uses a 256-byte lookup table for chunks of 8 bits/pixels instead of a 16-byte lookup table for chunks of 4 bits/pixels.
//! \author Geoffrey Anneheim, Lionel Debroux
void FastSpriteX8_MIRROR_H_R(unsigned short height asm("%d2"), unsigned short bytewidth asm("%d1"), char *sprt asm("%a0"), unsigned char *dest asm("%a1"));
//@}



// -----------------------------------------------------------------------------
/** @defgroup spritex8data Sprite data modification functions
 * \brief Miscellanous SpriteX8 functions: modify sprite data.
 */
// -----------------------------------------------------------------------------
/** @defgroup spritex8datawithmask Apply same mask to all lines
 * @ingroup spritex8data
 * The "withmask" functions perform a AND/OR/XOR operation of each line of the (\a bytewidth *8)x\a height sprite pointed to by \a src,
 * with the line pointed to by \a maskval, storing the result to the area pointed to by \a dest.
 * \since 2.00 Beta 5
 */
//@{
void SpriteX8Data_withmask_AND_R(unsigned short height asm("%d2"), unsigned short bytewidth asm("%d1"), const unsigned char *src asm("%a0"), const unsigned char *maskval asm("%a2"), unsigned char *dest asm("%a1"));
void SpriteX8Data_withmask_OR_R(unsigned short height asm("%d2"), unsigned short bytewidth asm("%d1"), const unsigned char *src asm("%a0"), const unsigned char *maskval asm("%a2"), unsigned char *dest asm("%a1"));
void SpriteX8Data_withmask_XOR_R(unsigned short height asm("%d2"), unsigned short bytewidth asm("%d1"), const unsigned char *src asm("%a0"), const unsigned char *maskval asm("%a2"), unsigned char *dest asm("%a1"));
//@}

/** @defgroup spritex8datawithsprite Modify sprite with another sprite of same size
 * @ingroup spritex8data
 * The "withsprite" functions perform a AND/OR/XOR operation of the whole (\a bytewidth *8)x\a height sprite pointed to by \a src1,
 * with the sprite of same size pointed to by \a src2, storing the result to the area pointed to by \a dest.
 * \since 2.00 Beta 5
 */
//@{
void SpriteX8Data_withsprite_AND_R(unsigned short height asm("%d2"), unsigned short bytewidth asm("%d1"), const unsigned char *src1 asm("%a0"), const unsigned char *src2 asm("%a2"), unsigned char *dest asm("%a1"));
void SpriteX8Data_withsprite_OR_R(unsigned short height asm("%d2"), unsigned short bytewidth asm("%d1"), const unsigned char *src1 asm("%a0"), const unsigned char *src2 asm("%a2"), unsigned char *dest asm("%a1"));
void SpriteX8Data_withsprite_XOR_R(unsigned short height asm("%d2"), unsigned short bytewidth asm("%d1"), const unsigned char *src1 asm("%a0"), const unsigned char *src2 asm("%a2"), unsigned char *dest asm("%a1"));
//@}



// -----------------------------------------------------------------------------
/** @defgroup rotatesprite Sprite rotation functions
 * \brief Arbitrary angle sprite rotating functions.
 *
 * Rotate clockwise (8|16|32|\a bytewidth *8)x\a height sprite pointed to by \a src of \a degreesClockwise degrees, and store the result as sprite data in area pointed to by \a dest.<br>
 * The center of rotation is (\a originX, \a originY).
 * \author Joey Adams
 * \since 2.00 Beta 5
 */
// -----------------------------------------------------------------------------
//@{
void RotateSprite8_R(const unsigned char *src asm("%a0"), unsigned char *dest asm("%a1"), unsigned short height, short originX asm("%d1"), short originY asm("%d2"), long degreesClockwise asm("%d0")) __attribute__((__stkparm__));
void RotateSprite16_R(const unsigned short *src asm("%a0"), unsigned short *dest asm("%a1"), unsigned short height, short originX asm("%d1"), short originY asm("%d2"), long degreesClockwise asm("%d0")) __attribute__((__stkparm__));
void RotateSprite32_R(const unsigned long *src asm("%a0"), unsigned long *dest asm("%a1"), unsigned short height, short originX asm("%d1"), short originY asm("%d2"), long degreesClockwise asm("%d0")) __attribute__((__stkparm__));
void RotateSpriteX8_R(const unsigned char *src asm("%a0"), unsigned char *dest asm("%a1"), unsigned short width, short height, short originX asm("%d1"), short originY asm("%d2"), long degreesClockwise asm("%d0")) __attribute__((__stkparm__));
//@}



//--END_FUNCTION_PROTOTYPES--//


// -----------------------------------------------------------------------------
//! Sine table (0..90 \htmlonly &deg; \endhtmlonly ) used in arbitrary angle sprite rotating functions, values multiplied by 8192 == 1<<13.
//! \author Joey Adams.
//! \since 2.00 Beta 5
extern const unsigned short RS_sin8192tab[91];
// Exported to avoid duplication of sine tables in case a program would use another sine table, good idea of Joey.



//=============================================================================
//=============================================================================
//=============================================================================
//
// ttunpack.h (now extgraph.h contains everything necessary for unpacking)
// taken from TIGCC Tools Suite
// Copyright (c) 2000-2004 TICT
//
//=============================================================================
//=============================================================================
//=============================================================================
#ifndef __TTUNPACK_H__
#define __TTUNPACK_H__

//! Structure describing the layout of a valid TTUnpack header.
typedef struct {
    unsigned char  osize_lo;   ///< original size lowbyte
    unsigned char  osize_hi;   ///< original size highbyte
    unsigned char  magic1;     ///< must be equal to TTUNPACK_MAGIC1
    unsigned char  magic2;     ///< must be equal to TTUNPACK_MAGIC2
    unsigned char  csize_lo;   ///< compressed size lowbyte
    unsigned char  csize_hi;   ///< compressed size highbyte
    unsigned char  esc1;       ///< escape >> (8-escBits)
    unsigned char  notused3;   ///< unused
    unsigned char  notused4;   ///< unused
    unsigned char  esc2;       ///< escBits
    unsigned char  gamma1;     ///< maxGamma + 1
    unsigned char  gamma2;     ///< (1<<maxGamma)
    unsigned char  extralz;    ///< extraLZPosBits
    unsigned char  notused1;   ///< unused
    unsigned char  notused2;   ///< unused
    unsigned char  rleentries; ///< rleUsed
} TTUNPACK_HEADER;

//! First byte ('t') of the ttunpack magic value
#define TTUNPACK_MAGIC1 0x54
//! Second byte ('p') of the ttunpack magic value
#define TTUNPACK_MAGIC2 0x50

//! Return values for \ref UnpackBuffer and \ref UnpackBufferGray functions.
//@{
#define TTUNPACK_OKAY             0
#define TTUNPACK_NOESCFOUND     248
#define TTUNPACK_ESCBITS        249
#define TTUNPACK_MAXGAMMA       250
#define TTUNPACK_EXTRALZP       251
#define TTUNPACK_NOMAGIC        252
#define TTUNPACK_OUTBUFOVERRUN  253
#define TTUNPACK_LZPOSUNDERRUN  254
//@}

/** @defgroup ttunpack TTUnpack handling macros and functions
 * \brief Decompression stuff. For more information, see the <a href="../../ExtGraph/exepack.html">ExePack/TTArchive documentation</a>.<br>
 * The current version of the ttunpack functions don't disable interrupts, it's up to the caller to do that if deemed
 * necessary (decompression is about 10% faster).
 */
//@{
//! TTunpack decompression function.
short UnpackBuffer(const unsigned char *src, unsigned char *dest) __attribute__((__stkparm__));
//! Name of the UnpackBuffer function in the old ttunpack.h.
#define ttunpack_decompress UnpackBuffer

//! \ref UnpackBufferGray is currently an alias of \ref UnpackBuffer.
#define UnpackBufferGray UnpackBuffer
//! Name of the UnpackBufferGray function in the old ttunpack.h.
#define ttunpack_decompress_gray UnpackBuffer

/** \brief TTunpack decompression function, smaller and slower flavour (same author, same functionality, different decompression routine size vs. decompression speed tradeoff).
 * \since 2.00 Beta 6
 * \note This routine has been in the ExtGraph repository for a long time... but for some reason, it was never exported in this header file, and it just didn't work the way it was declared (incorrect calling convention declaration).
 * \note This is the "small" version, not the "super duper small" version used in pstarter and ttstart: for compatibility with \a ttunpack_decompress, the "small" version has __stkparm__ calling convention, proper return value, and like \ref UnpackBuffer, \ref UnpackBufferSmall supports the rarely-used delta LZ mode of the compressor.
 */
short UnpackBufferSmall(const unsigned char *src, unsigned char *dest) __attribute__((__stkparm__));

//! Gets the size of the ttunpack data pointed to by \a _p_
#define ttunpack_size(_p_)  ((unsigned short)(((TTUNPACK_HEADER*)(_p_))->osize_lo | (((TTUNPACK_HEADER*)(_p_))->osize_hi << 8)))
//! Compares the magic number of the ttunpack header pointed to by \a _p_ against \ref TTUNPACK_MAGIC1 and \ref TTUNPACK_MAGIC2
#define ttunpack_valid(_p_) (((TTUNPACK_HEADER*)(_p_))->magic1 == TTUNPACK_MAGIC1 && ((TTUNPACK_HEADER*)(_p_))->magic2 == TTUNPACK_MAGIC2)
//@}

#else
#error extgraph.h already contains ttunpack.h defines (remove ttunpack.h include !)
#endif





//=============================================================================
//=============================================================================
//=============================================================================
//
// ttarchive.h (now extgraph.h contains everything for archive handling)
// taken from TIGCC Tools Suite
// Copyright (c) 2000-2004 TICT
//
//=============================================================================
//=============================================================================
//=============================================================================
#ifndef __TTARCHIVE_H__
#define __TTARCHIVE_H__

//! Structure describing the layout of an entry in a TTArchive.
typedef struct {
   unsigned short offset;     ///< offset to the entry data from end of entry list
   unsigned short length;     ///< length of entry
   char           name[8];    ///< entry name
   unsigned char  misc1[2];   ///< info from cfg file (may be queried by a program)
   unsigned char  misc2[2];   ///< info from cfg file (may be queried by a program)
} TTARCHIVE_ENTRY;

//! Structure describing the header of TTArchive file.
typedef struct {
   unsigned long   magic;    ///< must be equal to TTARCHIVE_MAGIC
   unsigned short  nr;       ///< number of entries
   TTARCHIVE_ENTRY entry[0]; ///< here comes the list of TTARCHIVE_ENTRY structures
} TTARCHIVE_HEADER;

//! TTArchive magic value
#define TTARCHIVE_MAGIC 0x74746100L

// -----------------------------------------------------------------------------
// smart macros to access ttarchive
//
// _p_   ... pointer to archive start address
// _i_   ... index of entry
//
// NOTE: No checking is done in the macros !!
// -----------------------------------------------------------------------------

//! @defgroup ttarchive TTArchive handling macros
//! For more information, see the <a href="../../ExtGraph/exepack.html">ExePack/TTArchive documentation</a>.
//@{
//! Compares the magic number of the ttarchive header pointed to by \a _p_ against \ref TTARCHIVE_MAGIC
#define ttarchive_valid(_p_)     (((TTARCHIVE_HEADER*)(_p_))->magic == TTARCHIVE_MAGIC)
//! Returns the number of entries of the ttarchive pointed to by \a _p_
#define ttarchive_entries(_p_)   (((TTARCHIVE_HEADER*)(_p_))->nr)
//! Returns a pointer to the descriptor of the \a _i_ th archive entry of the ttarchive pointed to by \a _p_
#define ttarchive_desc(_p_,_i_)  (&(((TTARCHIVE_HEADER*)(_p_))->entry[_i_]))
//! Returns a pointer to the actual data of the \a _i_ th archive entry of the ttarchive pointed to by \a _p_
#define ttarchive_data(_p_,_i_)  (((unsigned char*)&(((TTARCHIVE_HEADER*)(_p_))->entry[((TTARCHIVE_HEADER*)(_p_))->nr]))+\
                                 ((TTARCHIVE_HEADER*)(_p_))->entry[_i_].offset)

//! Gets the size of the ttarchive data pointed to by \a _p_
#define ttarchive_size(_p_)      ({TTARCHIVE_ENTRY* e=&(((TTARCHIVE_HEADER*)(_p_))->entry[((TTARCHIVE_HEADER*)(_p_))->nr-1]);\
                                  ((unsigned char*)e)+sizeof(TTARCHIVE_ENTRY)+e->offset+e->length-(unsigned char*)(_p_);})

//! Length of information string in a TTArchive
#define TTARCHIVE_INFOLENGTH 20

//! Retrieves the string describing the TTArchive pointed to by \a _p_
#define ttarchive_info(_p_)      ({char* p=((char*)(_p_))+ttarchive_size(_p_)+21;\
                                  ((!(*p) && (*(p+1) ==((char)0xad)))?(p-TTARCHIVE_INFOLENGTH):NULL);})
//@}

#else
#error extgraph.h already contains ttarchive.h defines (remove ttarchive.h include !)
#endif

#endif

//#############################################################################
// Revision History
//#############################################################################
//
// Huge changes for v2.00 (rewrites, internal organization of library, usage
// of Doxygen...).
// No longer using CVS, but using online SVN starting from nearly three years
// before 2.00 Beta 5.
// The changelog is in the documentation and will stay there.
//
// -- ExtGraph 2.xx --
//
//
// $Log: extgraph.h,v $
//
// Revision 1.13  2002/05/22 09:19:20  tnussb
// for TIGCC versions greater than 0.93 all functions are declared with
// "__attribute__((__stkparm__))" to work correctly with compilation
// switch -mregparm
//
// Revision 1.12  2002/05/08 19:32:46  tnussb
// version number raised (just a bug in unpack function was fixed)
//
// Revision 1.11  2002/04/05 13:47:28  tnussb
// changes for v1.00
//
// Revision 1.10  2002/04/02 18:10:35  tnussb
// version number raised
//
// Revision 1.9  2002/03/21 12:20:39  tnussb
// FastDrawVLine added and version number changed
//
// Revision 1.8  2002/02/25 17:01:01  tnussb
// ttunpack stuff moved in front of ttarchive stuff
//
// Revision 1.7  2002/02/25 13:16:42  tnussb
// ttarchive.h and ttunpack.h integrated
//
// Revision 1.6  2002/02/22 17:13:57  tnussb
// fixed double use of EXTGRAPH_VERSION_MAIN and EXTGRAPH_VERSION_SUB (thanx, Kevin!)
//
// Revision 1.5  2002/02/22 16:31:03  tnussb
// (1) version depended stuff added (like EXTGRAPH_VERSION_STR)
// (2) complete header revised
// (3) CheckHWMatch() and DESIRED_CALCTYPE added
//
// Revision 1.4  2002/02/11 13:43:54  tnussb
// version number raised
//
// Revision 1.3  2002/02/11 10:24:37  tnussb
// generic commit for v0.87
//
// Revision 1.2  2001/06/22 14:33:01  Thomas Nussbaumer
// grayscale sprite drawing routines added
//
// Revision 1.1  2001/06/20 21:34:08  Thomas Nussbaumer
// initial check-in
//
//

