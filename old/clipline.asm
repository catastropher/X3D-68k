; Assembly Source File
; Created 2/19/2015, 11:23:01 PM


; Assembly Source File
; Created 8/19/02, 10:49:18 PM
; inputs: d0.w=x1, d1.w=y1, d2.w=x2, d3.w=y2
; outputs: none
; destroys: all regs except d6, a2, and a7
; note: I suggest avoiding endpoints with coordinates that are greater than 15000 or less than -15000 (i sure hope you won't need lines that long!)
; this routine is designed for hw2 - i believe that if you wish to run it on an hw1 calc you will have to change all "4C00"s to the address of the screen on your calc

	xdef draw_clip_line

draw_clip_line:
	movem.l d3-d7/a0/a1/a3-a6,-(sp)
	jsr drawline
	movem.l (sp)+,d3-d7/a0/a1/a3-a6
	rts

;a2 = addr
drawline:
	tst.w	d0
	bpl	contcheck
	tst.w	d2
	bmi	donewhile
contcheck:
	tst.w	d1
	bpl	contcheck2
	tst.w	d3
	bmi	donewhile
contcheck2:
	cmpi.w	#240,d0
	bmi	contcheck3
	cmpi.w	#240,d2
	bpl	donewhile
contcheck3:
	cmpi.w	#128,d1
	bmi	contcheck4
	cmpi.w	#128,d3
	bpl	donewhile
contcheck4:
	move.w	#1,a3
	move.w	d2,d4
	sub.w	d0,d4
	bpl	posdx
	move.w	#-1,a3
	neg.w	d4
posdx:
;	asr.w	#1,d4
	move.w	d4,a5
	move.w	#1,a4
	move.w	d3,d4
	sub.w	d1,d4
;	move.w	d4,dy
	bpl	posdy
	move.w	#-1,a4
	neg.w	d4
posdy:
;	asr.w	#1,d4
	move.w	d4,a6
	cmp.w	a5,d4
	bpl	ayisbigger
	move.w	a5,d7
	asr.w	#1,d7
;	sub.w	d4,d7
	neg.w	d7
	add.w	a6,d7
	tst.w	d0
	bpl	xnotneg
	muls.w	d0,d4
	ext.l	d7
	sub.l	d4,d7
	clr.w	d0
	move.l	a5,d5
	ext.l	d5
	move.l	d5,a5
whiledpos:
	tst.l	d7
	ble	startwhile
	add.w	a4,d1
	sub.l	a5,d7
	bra	whiledpos
xnotneg:
	cmpi.w	#240,d0
	blt	startwhile
	move.w	d0,d5
	sub.w	#239,d5
	muls.w	d4,d5
	ext.l	d7
	add.l	d5,d7
	move.w	#239,d0
	move.l	a5,d5
	ext.l	d5
	move.l	d5,a5
whiledpos2:
	tst.l	d7
	ble	startwhile
	add.w	a4,d1
	sub.l	a5,d7
	bra	whiledpos
startwhile:
	tst.w	d0
	bmi	donewhile
	cmpi.w	#240,d0
	bpl	donewhile
	tst.w	d1
	bmi	nopix
	cmpi.w	#128,d1
	bpl	nopix
	move.l	a2,a0;#$4C00,a0
	move.w	d0,d4
	move.w	d1,d5
	asl.w	#5,d5
	sub.w	d1,d5
	sub.w	d1,d5
	asr.w	#3,d4
	add.w	d5,a0
	add.w	d4,a0
	move.w	d0,d4
	andi.w	#7,d4
	lea	pixelmasks(pc),a1
	add.w	d4,a1
	move.b	(a1),d5
	or.b	d5,(a0)
nopix:
	cmp.w	d0,d2
	beq	donewhile
	tst.w	d7
	ble	fixed7
	add.w	a4,d1
	sub.w	a5,d7
fixed7:
	add.w	a3,d0
	add.w	a6,d7
	bra	startwhile
ayisbigger:
	asr.w	#1,d4
;	move.w	ax,d7
;	sub.w	d4,d7
	move.w	d4,d7
	neg.w	d7
	add.w	a5,d7
	tst.w	d1
	bpl	ynotneg
	move.w	a5,d4
	muls.w	d1,d4
	ext.l	d7
	sub.l	d4,d7
	clr.w	d1
	move.l	a6,d5
	ext.l	d5
	move.l	d5,a6
whiled1:
	tst.l	d7
	ble	ygreaterwhile
	add.w	a3,d0
	sub.l	a6,d7
	bra	whiled1
ynotneg:
	cmpi.w	#128,d1
	bmi	ygreaterwhile
	move.w	d1,d4
	sub.w	#127,d4
	move.w	a5,d5
	muls.w	d5,d4
	ext.l	d7
	add.l	d4,d7
	move.w	#127,d1
	move.l	a6,d5
	ext.l	d5
	move.l	d5,a6
whiled1two:
	tst.w	d7
	ble	ygreaterwhile
	add.w	a3,d0
	sub.w	a6,d7
	bra	whiled1two
ygreaterwhile:
	tst.w	d1
	bmi	donewhile
	cmpi.w	#128,d1
	bpl	donewhile
	tst.w	d0
	bmi	nopix2
	cmpi.w	#240,d0
	bpl	nopix2
	move.l	a2,a0;#$4C00,a0
	move.w	d0,d4
	move.w	d1,d5
	asl.w	#5,d5
	sub.w	d1,d5
	sub.w	d1,d5
	asr.w	#3,d4
	add.w	d5,a0
	add.w	d4,a0
	move.w	d0,d4
	andi.w	#7,d4
	lea	pixelmasks(pc),a1
	add.w	d4,a1
	move.b	(a1),d5
	or.b	d5,(a0)
nopix2:
	cmp.w	d1,d3
	beq	donewhile
	tst.w	d7
	ble	fixed72
	add.w	a3,d0
	sub.w	a6,d7
fixed72:
	add.w	a4,d1
	add.w	a5,d7
	bra	ygreaterwhile
donewhile:
	rts
pixelmasks:
	dc.b	%10000000
	dc.b	%01000000
	dc.b	%00100000
	dc.b	%00010000
	dc.b	%00001000
	dc.b	%00000100
	dc.b	%00000010
	dc.b	%00000001