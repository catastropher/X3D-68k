// C Source File
// Created 2/22/2015; 12:01:56 PM

#include <tigcclib.h>

asm("
	.section	.text.XFilledTriangle_R,\"x\"
	.even
	.globl	FilledTriangle_R
	.globl	XGrayFilledTriangle_R
	.globl lcd_h
FilledTriangle_R:
XGrayFilledTriangle_R:
	movem.l %d3-%d7/%a3-%a6,-(%sp)
	movea.l 0xC8.w,%a3
	movea.l 0x2A8*4(%a3),%a3 | _ds32s32

| check for a degenerated triangle
| (y coordinates or x coordinates collapsed)
| if ((y1==y2 && y2==y3) || (x1==x2 && x2==x3)) return;
	cmp.w %d3,%d1
	bne.s .L2
	cmp.w %a1,%d1
	beq.w .L34
.L2:
	|cmp.w %d0,%d2
	|bne.s .L5
	|cmp.w %d2,%d4
	|beq.w .L34

| sort points by their y coordinate ...
| point with highest y will be afterwards in (x1/y1),
| point with lowest y will be in (x3/y3)

| if (y2>=y1) {
|     ASM_EXCHANGE(x1,x2);
|     ASM_EXCHANGE(y1,y2);
| }
.L5:
	cmp.w %d3,%d1
	bgt.s .L7
	exg %d0,%d2
	exg %d1,%d3

| if (y3>=y1) {
|     ASM_EXCHANGE(x1,x3);
|     ASM_EXCHANGE(y1,y3);
| }
.L7:
	cmp.w %a1,%d1
	bgt.s .L9
	exg %d0,%d4
	exg %d1,%a1

| if (y3>=y2) {
|     ASM_EXCHANGE(x2,x3);
|     ASM_EXCHANGE(y2,y3);
| }
.L9:
	cmp.w %a1,%d3
	bgt.s .L11
	exg %d2,%d4
	exg %d3,%a1

| x1 <<= SHIFTCOUNT;
| x2 <<= SHIFTCOUNT;
| x3 <<= SHIFTCOUNT;
.L11:
	swap %d0
	clr.w %d0
	swap %d2
	clr.w %d2
	swap %d4
	clr.w %d4

| y3 -> d5
	move.w %a1,%d5
| addr = (unsigned char *)plane+((((y3)+(y3))<<4)-((y3)+(y3)));
	adda.w %a1,%a1
	move.w %a1,%d6
	lsl.w #4,%d6
	sub.w %a1,%d6
	lea 0(%a0,%d6.w),%a5

| At this point, d6, d7, a0, a1, a3, a4, a6 are scratch registers.

| calculate dx/dy for highest-to-middle point
| if ((y=y1-y2)) m3 = (x1-x2)/y;
| else           m3 = 999;
| XXX is 999 still OK ?
	moveq #0,%d6
	move.w %d1,%d6
	sub.w %d3,%d6
	suba.l %a6,%a6
	beq.s .L15
	movem.l %d0-%d2,-(%sp)
	move.l %d0,%d1
	sub.l %d2,%d1
	move.l %d6,%d0
	jsr (%a3)
| Result is in d1
	movea.l %d1,%a6
	movem.l (%sp)+,%d0-%d2

| At this point, d6, d7, a0, a1, a4 are scratch registers.

| calculate dx/dy for highest-to-lowest point
| if ((y=y1-y3)) m1 = (x1-x3)/y;
| else           m1 = 999;
| XXX is 999 still OK ?
.L15:
	move.w %d1,%d6
	sub.w %d5,%d6
	suba.l %a4,%a4
	beq.s .L18
	movem.l %d0-%d2,-(%sp)
	move.l %d0,%d1
	sub.l %d4,%d1
	move.l %d6,%d0
	jsr (%a3)
| Result is in d1
	movea.l %d1,%a4
	movem.l (%sp)+,%d0-%d2

| At this point, d6, d7, a0, a1 are scratch registers.

| calculate dx/dy for middle-to-lowest point
| if ((y=y2-y3)) m2 = (x2-x3)/y;
| else           m2 = 999;
| XXX is 999 still OK ?
|
| start point is the lowest one point (xcoordinates shifted up by SHIFTCOUNT bits)
| xa=x3;
|
| if highest-and-middle lowest point share their xcoordinate xb is the xcoord of middle point
| if (m2==999) xb=x2;
| else         xb=xa;
.L18:
	move.w %d3,%d6
	sub.w %d5,%d6
	beq.s .L21
	move.l %d1,-(%sp)
	move.l %d2,-(%sp)
	move.l %d6,%d0
	move.l %d2,%d1
	sub.l %d4,%d1
	jsr (%a3)
| Result is in d1
	movea.l %d1,%a3
	move.l (%sp)+,%d2
	move.l (%sp)+,%d1
	move.l %d4,%d6
	bra.s .L23
.L21:
	suba.l %a3,%a3
	move.l %d2,%d6
.L23:

	move.w %d1,%d7


| x2 -> d2
| y2 -> d3
| xa (formerly x3) -> d4
| y3 -> d5 (will be used as y, since the iteration starts from y3)
| xb -> d6
| y1 -> d7 (used as an iteration bound for the bottom of the triangle)
| drawfunc -> a2
| m2 -> a3
| m1 -> a4
| addr -> a5
| m3 -> a6
| d0, d1, a1: scratch registers

| if (x2<(x3+(y2-y3)*m1)) {
|     xa += (1 << SHIFTCOUNT);
| }
| else {
|     xb += (1 << SHIFTCOUNT);
| }
| (approximated so as to avoid using a 32x32 multiplication routine)
	move.w %d3,%d0
	sub.w %d5,%d0
	move.l %a4,%d1
	swap %d1
	muls.w %d1,%d0
	swap %d0
	add.l %d4,%d0
	cmp.l %d2,%d0
	ble.s .L24
	swap %d4
	addq.w #1,%d4
	swap %d4
	bra.s add_here
.L24:
	swap %d6
	addq.w #1,%d6
	swap %d6
	
add_here:
	|if the lowest point > lcd_h, set it to lcd_h
	cmpi.w #127,%d7 | CHANGE LATER
	ble.s no_swap
	move.w #127,%d7
no_swap:
	|if the middle point > lcd_h, set it to lcd_h
	cmpi.w #127,%d3 | CHANGE LATER
	ble.s no_swap_2
	move.w #127,%d3
no_swap_2:


.L26:
	move.l %d4,%d0
	swap %d0
	move.l %d6,%d1
	swap %d1
	move.l %a5,%a0
	jsr (%a2)
	addq.w #1,%d5
	add.l %a4,%d4
	add.l %a3,%d6
	lea (30,%a5),%a5
.L27:
	cmp.w %d5,%d3
	bgt.s .L26
	bra.s .L35
.L30:
	move.l %d4,%d0
	swap %d0
	move.l %d6,%d1
	swap %d1
	move.l %a5,%a0
	jsr (%a2)
	addq.w #1,%d5
	add.l %a4,%d4
	add.l %a6,%d6
	lea (30,%a5),%a5
.L35:
	cmp.w %d5,%d7
	bge.s .L30
.L34:
	movem.l (%sp)+,%d3-%d7/%a3-%a6
	rts
");