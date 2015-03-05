; Assembly Source File
; Created 2/19/2015, 11:59:03 PM

;Assembly Source File
;Created 9/4/02, 3:49:31 PM by asmviper
;THIS FILE IS BEST VIEWED IN NOTEPAD WITH WORD WRAP ON
;destroys:	all regs, except a7
;inputs:	d0.w=x1, d1.w=y1, d2.w=x2, d3.w=y2, d4.w=x3, d5.w=y3 (have reasonable coordinates - I can't guarantee success if the coordinates have an absolute value greater than 4000)
;ouptuts:	none
;draws:	a filled, black, clipped triangle
;disclaimer: I AM NOT RESPONSIBLE FOR ANY DAMAGE THIS SOURCE CAUSES. I have done my best to make it glitch free, but nobody's perfect. It has been tested extensively on a HW2 Calc with AMS 2.05. If you have questions, comments, or suggestions, I would be happy to hear from you! My email is asmviper@hotmail.com

; a0 = screen addr

	xdef draw_clip_tri

draw_clip_tri:
	movem.l d6-d7/a1-a6,-(sp)
	jsr drawtri
	movem.l (sp)+,d6-d7/a1-a6
	rts
	

drawtri:
	move.l a0,(screen_addr)
	tst.w	d1
	bpl	someposy
	tst.w	d3
	bpl	someposy
	tst.w	d5
	bpl	someposy
	rts
someposy:
	cmp.w	#128,d1
	bmi	somenegy
	cmp.w	#128,d3
	bmi	somenegy
	cmp.w	#128,d5
	bmi	somenegy
	rts
somenegy:
	tst.w	d0
	bpl	someposx
	tst.w	d2
	bpl	someposx
	tst.w	d4
	bpl	someposx
	rts
someposx:
	cmp.w	#240,d0
	bmi	somenegx
	cmp.w	#240,d2
	bmi	somenegx
	cmp.w	#240,d4
	bmi	somenegx
	rts
somenegx:
	cmp.w	d1,d3
	bmi	d1bigger
	beq	maybehoriz
	exg.w	d1,d3
	exg.w	d0,d2
d1bigger:
	cmp.w	d1,d5
	ble	d1biggeragain
	exg.w	d1,d5
	exg.w	d0,d4
d1biggeragain:
	cmp.w	d3,d5
	ble	d3bigger
	exg.w	d3,d5
	exg.w	d2,d4
d3bigger:
	cmp.w	d0,d2
	beq	maybevert
novert:
	move.w	d1,yt1
	move.w	d3,yt2
	move.w	d4,a4
	move.w	d4,a5
	move.w	d2,xd3
	move.w	d3,d7
	sub.w	d5,d7
	bpl	absdone2
	neg.w	d7
absdone2:
	move.w	d7,xc2
	asl.w	#1,d7
	move.w	d7,ady23
	move.w	d1,d7
	sub.w	d3,d7
	bpl	absdone3
	neg.w	d7
absdone3:
	move.w	d7,xc3
	asl.w	#1,d7
	move.w	d7,ady12
	move.w	d0,d6
	move.w	#1,sx13
	sub.w	d4,d6
	bpl	absx
	neg.w	d6
	move.w	#-1,sx13
absx:
	asl.w	#1,d6
	move.w	d2,d7
	move.w	#1,sx23
	sub.w	d4,d7
	bpl	absx2
	move.w	#-1,sx23
	neg.w	d7
absx2:
	asl.w	#1,d7
	move.w	d7,adx23
	move.w	d0,d7
	move.w	#1,sx12
	sub.w	d2,d7
	bpl	absx3
	neg.w	d7
	move.w	#-1,sx12
absx3:
	asl.w	#1,d7
	move.w	d7,adx12
	move.w	d1,d7
	sub.w	d5,d7
	bpl	absdone
	neg.w	d7
absdone:
	move.w	d7,d4
	asl.w	#1,d7
	move.w	d7,ady13
	tst.w	d5
	bpl	noprobs
	neg.w	d5
	move.w	d5,yt3
	muls.w	d6,d5
	ext.l	d4
	add.l	d5,d4
	move.w	ady13,d7
	ext.l	d7
checkovercount:
	cmp.l	d7,d4
	ble	contabs
	sub.l	d7,d4
	add.w	sx13,a4
	bra	checkovercount
contabs:
	tst.w	yt2
	beq	contabs2
	bmi	d3above
	move.w	xc2,d3
	ext.l	d3
	move.w	yt3,d5
	muls.w	adx23,d5
	add.l	d5,d3
	move.w	ady23,d7
	ext.l	d7
checkover:
	cmp.l	d7,d3
	ble	contabs2
	sub.l	d7,d3
	add.w	sx23,a5
	bra	checkover
contabs2:
	move.w	d3,xc2
	clr.w	d5
	move.l	(screen_addr),a6
	move.w	yt2,d3
	beq	startit
	bra	continue
noprobs:
	move.w	d5,d7
	asl.w	#5,d7
	sub.w	d5,d7
	sub.w	d5,d7
	ext.l	d7
	add.l	(screen_addr),d7
	move.l	d7,a6
	cmp.w	d3,d5
	beq	startit
continue:
	move.w	adx23,d7
	cmp.w	d1,d3
	beq	flatbottom
	move.w	xc2,d3
forloop1:
	cmp.w	ady13,d4
	ble	nowhile
	sub.w	ady13,d4
	add.w	sx13,a4
	bra	forloop1
nowhile:
	cmp.w	ady23,d3
	ble	nowhile2
	sub.w	ady23,d3
	add.w	sx23,a5
	bra	nowhile
nowhile2:
	move.w	a4,d0
	move.w	a5,d1
	move.l	a6,a0
	bsr	horizline
	add.l	#30,a6
	addq.l	#1,d5
	cmp.w	#128,d5
	beq	quit
	add.w	d6,d4
	add.w	d7,d3
	cmp.w	yt2,d5
	bmi	forloop1
startit:
	move.w	xd3,a5
	move.w	xc3,d3
	move.w	adx12,d7
forloop2:
	cmp.w	ady13,d4
	ble	norepeat
	sub.w	ady13,d4
	add.w	sx13,a4
	bra	forloop2
norepeat:
	cmp.w	ady12,d3
	ble	norepeat2
	sub.w	ady12,d3
	add.w	sx12,a5
	bra	norepeat
norepeat2:
	move.w	a4,d0
	move.w	a5,d1
	move.l	a6,a0
	bsr	horizline
	add.l	#30,a6
	addq.l	#1,d5
	cmp.w	#128,d5
	beq	quit
	add.w	d6,d4
	add.w	d7,d3
	cmp.w	yt1,d5
	ble	forloop2
	rts
horizline:
	cmp.w	d0,d1
	bpl	noswitch
	exg.w	d0,d1
noswitch:
	tst.w	d1
	bmi	quit
	cmp.w	#240,d0
	bpl	quit
	tst.w	d0
	bpl	conthoriz
	clr.w	d0
conthoriz:
	cmpi.w	#240,d1
	bmi	conthoriz2
	move.w	#239,d1
conthoriz2:
	move.l	a0,a3
	move.w	d0,d2
	asr.w	#3,d2
	ext.l	d2
	add.l	d2,a0
	move.w	d1,d2
	asr.w	#3,d2
	ext.l	d2
	add.l	d2,a3
	andi.w	#7,d0
	andi.w	#7,d1
	ext.l	d0
	ext.l	d1
	cmp.l	a0,a3
	beq	samebyte
	lea	leftable(pc),a1
	move.l	a1,a2
	add.l	#8,a2
	add.l	d0,a1
	move.b	(a1),d2
	and.b	d2,(a0)+	;HERE
	cmp.l	a3,a0
	beq	finishedloop
loop:
	sf.b	(a0)+
	cmp.l	a3,a0
	bne	loop
finishedloop:
	add.l	d1,a2
	move.b	(a2),d2
	and.b	d2,(a0)		;HERE
quit:
	rts
samebyte:
	eor.b	#111,d0		;HERE
	eor.b	#111,d1
	bset.b	d1,(a0)
	cmp.b	d1,d0
	beq	quit
overagain:
	addq.l	#1,d1
	bset.b	d1,(a0)
	cmp.b	d1,d0
	bne	overagain
	rts
maybehoriz:
	cmp.w	d1,d5
	bne	d1bigger
	cmp.w	d0,d4
	bpl	d0smaller
	exg.w	d0,d4
d0smaller:
	cmp.w	d0,d2
	bpl	d0smalleragain
	exg.w	d0,d2
d0smalleragain:
	cmp.w	d4,d2
	bpl	d2smaller
	exg.w	d2,d4
d2smaller:
	exg.w	d2,d1
	move.w	d2,d6
	asl.w	#5,d6
	sub.w	d2,d6
	sub.w	d2,d6
	ext.l	d6
	add.l	(screen_addr),d6
	move.l	d6,a0
	bsr	horizline
	rts
maybevert:
	cmp.w	d0,d4
	bne	novert
	cmpi.w	#128,d1
	bmi	d1done
	moveq.l	#127,d1
d1done:
	move.w	d1,d6
	asl.w	#5,d6
	sub.w	d1,d6
	sub.w	d1,d6
	asr.w	#3,d0
	add.w	d0,d6
	ext.l	d6
	add.l	(screen_addr),d6
	move.l	d6,a6
	not.b	d2
	andi.b	#7,d2
	move.w	d1,d3
	ext.l	d3
vertloop:
	bset.b	d2,(a6)
	cmp.w	d5,d3
	beq	quit
	lea	-30(a6),a6
	subq.l	#1,d3
	bpl	vertloop
	rts
flatbottom:
	move.w	ady23,ady12
	move.w	sx23,sx12
	move.w	xc2,d3
	bra	forloop2
d3above:
	move.w	xc3,d3
	ext.l	d3
	move.w	yt2,d5
	neg.w	d5
	muls.w	adx12,d5
	add.l	d5,d3
	move.w	ady12,d7
	ext.l	d7
	move.w	xd3,a5
d3aboveloop:
	cmp.l	d7,d3
	ble	d3abs
	sub.l	d7,d3
	add.w	sx12,a5
	bra	d3aboveloop
d3abs:
	move.w	adx12,d7
	clr.w	d5
	move.l	(screen_addr),a6
	bra	forloop2
yt1:	dc.w	0
yt2:	dc.w	0
yt3:	dc.w	0
xd3:	dc.w	0
xc2:	dc.w	0
xc3:	dc.w	0
ady13:	dc.w	0
ady23:	dc.w	0
ady12:	dc.w	0
adx23:	dc.w	0
adx12:	dc.w	0
sx13:	dc.w	0
sx23:	dc.w	0
sx12:	dc.w	0
rightable:
	dc.b	%11111111
	dc.b	%01111111
	dc.b	%00111111
	dc.b	%00011111
	dc.b	%00001111
	dc.b	%00000111
	dc.b	%00000011
	dc.b	%00000001
leftable:
	dc.b	%10000000
	dc.b	%11000000
	dc.b	%11100000
	dc.b	%11110000
	dc.b	%11111000
	dc.b	%11111100
	dc.b	%11111110
	dc.b	%11111111
	
screen_addr:
	dc.l 0
	
