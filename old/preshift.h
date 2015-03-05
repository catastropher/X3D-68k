//*****************************************************************************
/** \file preshift.h
 * \brief This file contains definitions for the preshifted sprites part of the ExtGraph library.
 *
 * ExtGraph is a compile-time library which contains <b>speed-optimized graphics
 * functions</b> for the TIGCC cross-compiler for TI-89, TI-89T, TI-92+ and TI-V200
 * (collectively known as TI-68k calculators).
 *
 * <b>Preshifted sprite support</b>:
 * <ul><li>helper functions to create <b>semi-preshifted</b> sprites from B/W and gray non-masked interlaced 8x8 and 16x16 sprites;</li>
 * <li>non-clipped OR and XOR macros (to save branches) with completely unrolled looops drawing those semi-preshifted sprites.</li></ul>
 * 
 * <i>Semi-preshifted</i> means that only each other shifting is precomputed. This of course decreases average brute force performance if
 * preshifted sprites are displayed at odd coordinates (not if they're displayed at even coordinates), but:
 * <ul><li>semi-preshifted sprites remain significantly faster than equivalent GraySpriteX functions on average;</li>
 * <li>completely preshifted sprites take an insane amount of memory (2K per 16x16 preshifted grayscale sprite), which makes them merely
 * unusable (only 31 16x16 preshifted grayscale sprites per 65518 bytes block !).</li></ul>
 * Semi-preshifted sprites allow for 63 preshifted 16x16 grayscale sprites per 65518 bytes block.
 *
 * Anyway, as you can see in <a href="../../ExtGraph/comparison.html">the comparison of different approaches</a>, games that just couldn't be
 * done with normal sprites or a tilemap engine are scarce. Think that using simpler modes + background save / restore can be smarter than
 * always redrawing everything.

 * \note Credits go to PpHd for the movem.l (sprt)+,d0-d7; or.l d0...d7,(dest)+ trick, taken in GenLib. It is roughly as fast as
 * repeated 2-instruction move / or sequences, with a simple test code (it might be slightly slower with real-life programs, since it
 * clobbers more registers, I don't know), but it is significantly smaller.
 * 
 * \todo Make and export 8x8 preshifted sprite functions. Keep in mind that "semi-preshifted 8x8 sprites are written so as to take advantage
 * of 1 word being enough for 4 shiftings out of 8. This allows for 170 preshifted 8x8 grayscale sprites per 65518 bytes block."
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

#ifndef __EXT_PRESHIFT_H__
#define __EXT_PRESHIFT_H__

#include <gray.h>   // necessary for GrayGetPlane() macros in the grayscale
                    // support routines !!

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
///! \defgroup preshift Semi-preshifted sprites support
//@{
//! Size of the buffer for a 8x8 semi-preshifted B/W sprite.
#define SIZE_OF_PSPRITE8x8    (1*(8*(4*2+4*4)))
//! Size of the buffer for a 8x8 semi-preshifted grayscale sprite.
#define SIZE_OF_PGSPRITE8x8   (2*SIZE_OF_PGSPRITE8x8)
//! Size of the buffer for a 16x16 semi-preshifted B/W sprite.
#define SIZE_OF_PSPRITE16x16  (1*(16*(8*4)))
//! Size of the buffer for a 16x16 semi-preshifted grayscale sprite.
#define SIZE_OF_PGSPRITE16x16 (2*SIZE_OF_PSPRITE16x16)
//--BEGIN_FUNCTION_PROTOTYPES--//
//extern void PreshiftSprite8x8(register const unsigned char *src asm("%a0"),register unsigned long *dest asm("%a1"));
//! Preshift 16x16 B/W sprite pointed to by \a src to buffer of size \ref SIZE_OF_PSPRITE16x16 pointed to by \a dest.
void PreshiftSprite16x16(register const unsigned short *src asm("%a0"),register unsigned long *dest asm("%a1"));
//extern void PreshiftGrayISprite8x8(register unsigned char *src asm("%a0"),register unsigned long *dest asm("%a1"));
//! Preshift 16x16 grayscale sprite pointed to by \a src to buffer of size \ref SIZE_OF_PGSPRITE16x16 pointed to by \a dest.
void PreshiftGrayISprite16x16(register const unsigned short *src asm("%a0"),register unsigned long *dest asm("%a1"));
//--END_FUNCTION_PROTOTYPES--//

//! OR the 16x16 semi-preshifted B/W sprite pointed to by \a sprt at (\a x, \a y) in 240-pixel-wide plane \a dest.
#define PSprite16x16_OR_R(x,y,sprt,dest)\
asm("movem.l %%d5-%%d7,-(%%sp);\
    move.w   %0,%%d0;\
    move.w   %1,%%d1;\
    move.l   %2,%%a1;\
    move.l   %3,%%a0;\
    move.w   %%d1,%%d2;\
    lsl.w    #4,%%d1;\
    sub.w    %%d2,%%d1;\
    move.w   %%d0,%%d2;\
    lsr.w    #4,%%d2;\
    add.w    %%d2,%%d1;\
    add.w    %%d1,%%d1;\
    adda.w   %%d1,%%a0;\
    move.w   %%d0,%%d2;\
    andi.w   #0xE,%%d0;\
    lsl.w    #5,%%d0;\
    adda.w   %%d0,%%a1;\
    andi.w   #0x1,%%d2;\
    bne.s    2f;\
1:\
    movem.l  (%%a1)+,%%d0-%%d7;\
    or.l     %%d0,(%%a0);\
    or.l     %%d1,30(%%a0);\
    or.l     %%d2,60(%%a0);\
    or.l     %%d3,90(%%a0);\
    or.l     %%d4,120(%%a0);\
    or.l     %%d5,150(%%a0);\
    or.l     %%d6,180(%%a0);\
    or.l     %%d7,210(%%a0);\
    movem.l  (%%a1)+,%%d0-%%d7;\
    or.l     %%d0,240(%%a0);\
    or.l     %%d1,270(%%a0);\
    or.l     %%d2,300(%%a0);\
    or.l     %%d3,330(%%a0);\
    or.l     %%d4,360(%%a0);\
    or.l     %%d5,390(%%a0);\
    or.l     %%d6,420(%%a0);\
    or.l     %%d7,450(%%a0);\
    bra.s    0f;\
2:\
    movem.l  (%%a1)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    or.l     %%d0,(%%a0);\
    ror.l    #1,%%d1;\
    or.l     %%d1,30(%%a0);\
    ror.l    #1,%%d2;\
    or.l     %%d2,60(%%a0);\
    ror.l    #1,%%d3;\
    or.l     %%d3,90(%%a0);\
    ror.l    #1,%%d4;\
    or.l     %%d4,120(%%a0);\
    ror.l    #1,%%d5;\
    or.l     %%d5,150(%%a0);\
    ror.l    #1,%%d6;\
    or.l     %%d6,180(%%a0);\
    ror.l    #1,%%d7;\
    or.l     %%d7,210(%%a0);\
    movem.l  (%%a1)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    or.l     %%d0,240(%%a0);\
    ror.l    #1,%%d1;\
    or.l     %%d1,270(%%a0);\
    ror.l    #1,%%d2;\
    or.l     %%d2,300(%%a0);\
    ror.l    #1,%%d3;\
    or.l     %%d3,330(%%a0);\
    ror.l    #1,%%d4;\
    or.l     %%d4,360(%%a0);\
    ror.l    #1,%%d5;\
    or.l     %%d5,390(%%a0);\
    ror.l    #1,%%d6;\
    or.l     %%d6,420(%%a0);\
    ror.l    #1,%%d7;\
    or.l     %%d7,450(%%a0);\
0:\
    movem.l  (%%sp)+,%%d5-%%d7;\
" : : "d"(x),"d"(y),"a"(sprt),"a"(dest) : "d0","d1","d2","d3","d4","a0","a1","cc");

//! XOR the 16x16 semi-preshifted B/W sprite pointed to by \a sprt at (\a x, \a y) in 240-pixel-wide plane \a dest.
#define PSprite16x16_XOR_R(x,y,sprt,dest)\
asm("movem.l %%d5-%%d7,-(%%sp);\
    move.w   %0,%%d0;\
    move.w   %1,%%d1;\
    move.l   %2,%%a1;\
    move.l   %3,%%a0;\
    move.w   %%d1,%%d2;\
    lsl.w    #4,%%d1;\
    sub.w    %%d2,%%d1;\
    move.w   %%d0,%%d2;\
    lsr.w    #4,%%d2;\
    add.w    %%d2,%%d1;\
    add.w    %%d1,%%d1;\
    adda.w   %%d1,%%a0;\
    move.w   %%d0,%%d2;\
    andi.w   #0xE,%%d0;\
    lsl.w    #5,%%d0;\
    adda.w   %%d0,%%a1;\
    andi.w   #0x1,%%d2;\
    bne.s    2f;\
1:\
    movem.l  (%%a1)+,%%d0-%%d7;\
    eor.l    %%d0,(%%a0);\
    eor.l    %%d1,30(%%a0);\
    eor.l    %%d2,60(%%a0);\
    eor.l    %%d3,90(%%a0);\
    eor.l    %%d4,120(%%a0);\
    eor.l    %%d5,150(%%a0);\
    eor.l    %%d6,180(%%a0);\
    eor.l    %%d7,210(%%a0);\
    movem.l  (%%a1)+,%%d0-%%d7;\
    eor.l    %%d0,240(%%a0);\
    eor.l    %%d1,270(%%a0);\
    eor.l    %%d2,300(%%a0);\
    eor.l    %%d3,330(%%a0);\
    eor.l    %%d4,360(%%a0);\
    eor.l    %%d5,390(%%a0);\
    eor.l    %%d6,420(%%a0);\
    eor.l    %%d7,450(%%a0);\
    bra.s    0f;\
2:\
    movem.l  (%%a1)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    eor.l    %%d0,(%%a0);\
    ror.l    #1,%%d1;\
    eor.l    %%d1,30(%%a0);\
    ror.l    #1,%%d2;\
    eor.l    %%d2,60(%%a0);\
    ror.l    #1,%%d3;\
    eor.l    %%d3,90(%%a0);\
    ror.l    #1,%%d4;\
    eor.l    %%d4,120(%%a0);\
    ror.l    #1,%%d5;\
    eor.l    %%d5,150(%%a0);\
    ror.l    #1,%%d6;\
    eor.l    %%d6,180(%%a0);\
    ror.l    #1,%%d7;\
    eor.l    %%d7,210(%%a0);\
    movem.l  (%%a1)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    eor.l    %%d0,240(%%a0);\
    ror.l    #1,%%d1;\
    eor.l    %%d1,270(%%a0);\
    ror.l    #1,%%d2;\
    eor.l    %%d2,300(%%a0);\
    ror.l    #1,%%d3;\
    eor.l    %%d3,330(%%a0);\
    ror.l    #1,%%d4;\
    eor.l    %%d4,360(%%a0);\
    ror.l    #1,%%d5;\
    eor.l    %%d5,390(%%a0);\
    ror.l    #1,%%d6;\
    eor.l    %%d6,420(%%a0);\
    ror.l    #1,%%d7;\
    eor.l    %%d7,450(%%a0);\
0:\
    movem.l  (%%sp)+,%%d5-%%d7;\
" : : "d"(x),"d"(y),"a"(sprt),"a"(dest) : "d0","d1","d2","d3","d4","a0","a1","cc");


//! OR the 16x16 semi-preshifted Bgrayscale sprite pointed to by \a sprt at (\a x, \a y) in 240-pixel-wide planes \a dest and \a dest1.
#define GrayPSprite16x16_OR_R(x,y,sprt,dest0,dest1)\
asm("movem.l %%d5-%%d7,-(%%sp);\
    move.w   %0,%%d0;\
    move.w   %1,%%d1;\
    move.l   %2,%%a2;\
    move.l   %3,%%a0;\
    move.l   %4,%%a1;\
    move.w   %%d1,%%d2;\
    lsl.w    #4,%%d1;\
    sub.w    %%d2,%%d1;\
    move.w   %%d0,%%d2;\
    lsr.w    #4,%%d2;\
    add.w    %%d2,%%d1;\
    add.w    %%d1,%%d1;\
    adda.w   %%d1,%%a0;\
    adda.w   %%d1,%%a1;\
    move.w   %%d0,%%d2;\
    andi.w   #0xE,%%d0;\
    lsl.w    #6,%%d0;\
    adda.w   %%d0,%%a2;\
    andi.w   #0x1,%%d2;\
    bne      2f;\
1:\
    movem.l  (%%a2)+,%%d0-%%d7;\
    or.l     %%d0,(%%a0);\
    or.l     %%d1,(%%a1);\
    or.l     %%d2,30(%%a0);\
    or.l     %%d3,30(%%a1);\
    or.l     %%d4,60(%%a0);\
    or.l     %%d5,60(%%a1);\
    or.l     %%d6,90(%%a0);\
    or.l     %%d7,90(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    or.l     %%d0,120(%%a0);\
    or.l     %%d1,120(%%a1);\
    or.l     %%d2,150(%%a0);\
    or.l     %%d3,150(%%a1);\
    or.l     %%d4,180(%%a0);\
    or.l     %%d5,180(%%a1);\
    or.l     %%d6,210(%%a0);\
    or.l     %%d7,210(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    or.l     %%d0,240(%%a0);\
    or.l     %%d1,240(%%a1);\
    or.l     %%d2,270(%%a0);\
    or.l     %%d3,270(%%a1);\
    or.l     %%d4,300(%%a0);\
    or.l     %%d5,300(%%a1);\
    or.l     %%d6,330(%%a0);\
    or.l     %%d7,330(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    or.l     %%d0,360(%%a0);\
    or.l     %%d1,360(%%a1);\
    or.l     %%d2,390(%%a0);\
    or.l     %%d3,390(%%a1);\
    or.l     %%d4,420(%%a0);\
    or.l     %%d5,420(%%a1);\
    or.l     %%d6,450(%%a0);\
    or.l     %%d7,450(%%a1);\
    bra      0f;\
2:\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    or.l     %%d0,(%%a0);\
    ror.l    #1,%%d1;\
    or.l     %%d1,(%%a1);\
    ror.l    #1,%%d2;\
    or.l     %%d2,30(%%a0);\
    ror.l    #1,%%d3;\
    or.l     %%d3,30(%%a1);\
    ror.l    #1,%%d4;\
    or.l     %%d4,60(%%a0);\
    ror.l    #1,%%d5;\
    or.l     %%d5,60(%%a1);\
    ror.l    #1,%%d6;\
    or.l     %%d6,90(%%a0);\
    ror.l    #1,%%d7;\
    or.l     %%d7,90(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    or.l     %%d0,120(%%a0);\
    ror.l    #1,%%d1;\
    or.l     %%d1,120(%%a1);\
    ror.l    #1,%%d2;\
    or.l     %%d2,150(%%a0);\
    ror.l    #1,%%d3;\
    or.l     %%d3,150(%%a1);\
    ror.l    #1,%%d4;\
    or.l     %%d4,180(%%a0);\
    ror.l    #1,%%d5;\
    or.l     %%d5,180(%%a1);\
    ror.l    #1,%%d6;\
    or.l     %%d6,210(%%a0);\
    ror.l    #1,%%d7;\
    or.l     %%d7,210(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    or.l     %%d0,240(%%a0);\
    ror.l    #1,%%d1;\
    or.l     %%d1,240(%%a1);\
    ror.l    #1,%%d2;\
    or.l     %%d2,270(%%a0);\
    ror.l    #1,%%d3;\
    or.l     %%d3,270(%%a1);\
    ror.l    #1,%%d4;\
    or.l     %%d4,300(%%a0);\
    ror.l    #1,%%d5;\
    or.l     %%d5,300(%%a1);\
    ror.l    #1,%%d6;\
    or.l     %%d6,330(%%a0);\
    ror.l    #1,%%d7;\
    or.l     %%d7,330(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    or.l     %%d0,360(%%a0);\
    ror.l    #1,%%d1;\
    or.l     %%d1,360(%%a1);\
    ror.l    #1,%%d2;\
    or.l     %%d2,390(%%a0);\
    ror.l    #1,%%d3;\
    or.l     %%d3,390(%%a1);\
    ror.l    #1,%%d4;\
    or.l     %%d4,420(%%a0);\
    ror.l    #1,%%d5;\
    or.l     %%d5,420(%%a1);\
    ror.l    #1,%%d6;\
    or.l     %%d6,450(%%a0);\
    ror.l    #1,%%d7;\
    or.l     %%d7,450(%%a1);\
0:\
    movem.l  (%%sp)+,%%d5-%%d7;\
" : : "d"(x),"d"(y),"a"(sprt),"a"(dest0),"a"(dest1) : "d0","d1","d2","d3","d4","a0","a1","a2","cc");

//! XOR the 16x16 semi-preshifted Bgrayscale sprite pointed to by \a sprt at (\a x, \a y) in 240-pixel-wide planes \a dest and \a dest1.
#define GrayPSprite16x16_XOR_R(x,y,sprt,dest0,dest1)\
asm("movem.l %%d5-%%d7,-(%%sp);\
    move.w   %0,%%d0;\
    move.w   %1,%%d1;\
    move.l   %2,%%a2;\
    move.l   %3,%%a0;\
    move.l   %4,%%a1;\
    move.w   %%d1,%%d2;\
    lsl.w    #4,%%d1;\
    sub.w    %%d2,%%d1;\
    move.w   %%d0,%%d2;\
    lsr.w    #4,%%d2;\
    add.w    %%d2,%%d1;\
    add.w    %%d1,%%d1;\
    adda.w   %%d1,%%a0;\
    adda.w   %%d1,%%a1;\
    move.w   %%d0,%%d2;\
    andi.w   #0xE,%%d0;\
    lsl.w    #6,%%d0;\
    adda.w   %%d0,%%a2;\
    andi.w   #0x1,%%d2;\
    bne      2f;\
1:\
    movem.l  (%%a2)+,%%d0-%%d7;\
    eor.l    %%d0,(%%a0);\
    eor.l    %%d1,(%%a1);\
    eor.l    %%d2,30(%%a0);\
    eor.l    %%d3,30(%%a1);\
    eor.l    %%d4,60(%%a0);\
    eor.l    %%d5,60(%%a1);\
    eor.l    %%d6,90(%%a0);\
    eor.l    %%d7,90(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    eor.l    %%d0,120(%%a0);\
    eor.l    %%d1,120(%%a1);\
    eor.l    %%d2,150(%%a0);\
    eor.l    %%d3,150(%%a1);\
    eor.l    %%d4,180(%%a0);\
    eor.l    %%d5,180(%%a1);\
    eor.l    %%d6,210(%%a0);\
    eor.l    %%d7,210(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    eor.l    %%d0,240(%%a0);\
    eor.l    %%d1,240(%%a1);\
    eor.l    %%d2,270(%%a0);\
    eor.l    %%d3,270(%%a1);\
    eor.l    %%d4,300(%%a0);\
    eor.l    %%d5,300(%%a1);\
    eor.l    %%d6,330(%%a0);\
    eor.l    %%d7,330(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    eor.l    %%d0,360(%%a0);\
    eor.l    %%d1,360(%%a1);\
    eor.l    %%d2,390(%%a0);\
    eor.l    %%d3,390(%%a1);\
    eor.l    %%d4,420(%%a0);\
    eor.l    %%d5,420(%%a1);\
    eor.l    %%d6,450(%%a0);\
    eor.l    %%d7,450(%%a1);\
    bra      0f;\
2:\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    eor.l    %%d0,(%%a0);\
    ror.l    #1,%%d1;\
    eor.l    %%d1,(%%a1);\
    ror.l    #1,%%d2;\
    eor.l    %%d2,30(%%a0);\
    ror.l    #1,%%d3;\
    eor.l    %%d3,30(%%a1);\
    ror.l    #1,%%d4;\
    eor.l    %%d4,60(%%a0);\
    ror.l    #1,%%d5;\
    eor.l    %%d5,60(%%a1);\
    ror.l    #1,%%d6;\
    eor.l    %%d6,90(%%a0);\
    ror.l    #1,%%d7;\
    eor.l    %%d7,90(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    eor.l    %%d0,120(%%a0);\
    ror.l    #1,%%d1;\
    eor.l    %%d1,120(%%a1);\
    ror.l    #1,%%d2;\
    eor.l    %%d2,150(%%a0);\
    ror.l    #1,%%d3;\
    eor.l    %%d3,150(%%a1);\
    ror.l    #1,%%d4;\
    eor.l    %%d4,180(%%a0);\
    ror.l    #1,%%d5;\
    eor.l    %%d5,180(%%a1);\
    ror.l    #1,%%d6;\
    eor.l    %%d6,210(%%a0);\
    ror.l    #1,%%d7;\
    eor.l    %%d7,210(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    eor.l    %%d0,240(%%a0);\
    ror.l    #1,%%d1;\
    eor.l    %%d1,240(%%a1);\
    ror.l    #1,%%d2;\
    eor.l    %%d2,270(%%a0);\
    ror.l    #1,%%d3;\
    eor.l    %%d3,270(%%a1);\
    ror.l    #1,%%d4;\
    eor.l    %%d4,300(%%a0);\
    ror.l    #1,%%d5;\
    eor.l    %%d5,300(%%a1);\
    ror.l    #1,%%d6;\
    eor.l    %%d6,330(%%a0);\
    ror.l    #1,%%d7;\
    eor.l    %%d7,330(%%a1);\
    movem.l  (%%a2)+,%%d0-%%d7;\
    ror.l    #1,%%d0;\
    eor.l    %%d0,360(%%a0);\
    ror.l    #1,%%d1;\
    eor.l    %%d1,360(%%a1);\
    ror.l    #1,%%d2;\
    eor.l    %%d2,390(%%a0);\
    ror.l    #1,%%d3;\
    eor.l    %%d3,390(%%a1);\
    ror.l    #1,%%d4;\
    eor.l    %%d4,420(%%a0);\
    ror.l    #1,%%d5;\
    eor.l    %%d5,420(%%a1);\
    ror.l    #1,%%d6;\
    eor.l    %%d6,450(%%a0);\
    ror.l    #1,%%d7;\
    eor.l    %%d7,450(%%a1);\
0:\
    movem.l  (%%sp)+,%%d5-%%d7;\
" : : "d"(x),"d"(y),"a"(sprt),"a"(dest0),"a"(dest1) : "d0","d1","d2","d3","d4","a0","a1","a2","cc");

//@}

#endif
