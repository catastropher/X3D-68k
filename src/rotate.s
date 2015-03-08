| Assembly Source File
| Created 2/17/2015; 12:15:30 AM

	.xdef mul_fps
	.xdef mul_fps_frac
	.xdef asm_rotate_vex3d

mul_fps:
	muls.w %d1,%d0
	asr.l #8,%d0
	rts
	
mul_fps_frac:
	muls.w %d1,%d0
	asl.l #1,%d0
	swap %d0
	rts
	
.macro LOAD_VEC3 src
	movem.w (\src)+,%d0-%d2
.endm

.macro SET_VEC3_ROT src,dest
	
	move.l \dest,\src
.endm

|loads the first 6 elements of a matrix into d3-d7 and a4
.macro LOAD_MAT3_FIRST
	movem.w (%a5)+,%d3-%d7/%a4
.endm

|=================================================================
|save the d registers (d0 - d7)
|=================================================================
.macro SAVE_DR 
	movem.l %d0-%d7,-(%sp) 
.endm

|=================================================================
|restores the d registers (d7 - d0)
|=================================================================
.macro RESTORE_DR
	movem.l (%sp)+,%d0-%d7
.endm


|=================================================================
|multiplies a mat3_local by a mat3_gloabal (concatenates them)
|inputs:	a2 = mat3_local (mat1), a3 = mat3_global (mat2), 
|		a4 = mat3_local dest
|=================================================================
mul_mat3:
	|MAT1 ROW 1
	movem.l (%a2)+,%d0-%d2	|load in the first row of mat1
	movem.w (%a3)+,%d3-%d5	|load in the first row of mat2

	muls.w %d0,%d3	|multiply the rows
	muls.w %d1,%d4
	muls.w %d2,%d5
	
	add.l %d4,%d3	|sum the products
	add.l %d5,%d3
	asl.l #2,%d3	|correct the number of fractional bits
	move.l %d3,%a6
	
	|move.l %d3,%(
	
	movem.w (%a3)+,%d3-%d5	|load in the second row of mat2
	
	muls.w %d0,%d3	|multiply the rows
	muls.w %d1,%d4
	muls.w %d2,%d5
	
	add.l %d4,%d3	|sum the products
	add.l %d5,%d3
	asl.l #2,%d3	|correct the number of fractional bits
	move.l %d3,%a5
	
	movem.w (%a3)+,%d3-%d5	|load in the third row of mat2
	
	muls.w %d0,%d3	|multiply the rows
	muls.w %d1,%d4
	muls.w %d2,%d5
	
	add.l %d4,%d3	|sum the products
	add.l %d5,%d3
	asl.l #2,%d3	|correct the number of fractional bits
	move.l %d3,%a1
	
	|MAT1 ROW 2
	
	movem.l (%a2)+,%d0-%d2	|load in the second row of mat1
	movem.w (%a3)+,%d3-%d5	|load in the first row of mat2

	muls.w %d0,%d3	|multiply the rows
	muls.w %d1,%d4
	muls.w %d2,%d5
	
	add.l %d4,%d3	|sum the products
	add.l %d5,%d3
	asl.l #2,%d3	|correct the number of fractional bits
	move.l %d3,%a6
	
	movem.w (%a3)+,%d3-%d5	|load in the second row of mat2
	
	muls.w %d0,%d3	|multiply the rows
	muls.w %d1,%d4
	muls.w %d2,%d5
	
	add.l %d4,%d3	|sum the products
	add.l %d5,%d3
	asl.l #2,%d3	|correct the number of fractional bits
	move.l %d3,%a5
	
	movem.w (%a3)+,%d3-%d5	|load in the third row of mat2
	
	muls.w %d0,%d3	|multiply the rows
	muls.w %d1,%d4
	muls.w %d2,%d5
	
	add.l %d4,%d3	|sum the products
	add.l %d5,%d3
	asl.l #2,%d3	|correct the number of fractional bits
	move.l %d3,%a1		
	




	

|a3 = output vec3
|a4 = input vec3
|a5 = matrix
asm_rotate_vex3d:
	movem.l %d1-%d7,-(%sp)
	
	|load the input vector into registers d0-d2
	movem.w (%a4)+,%d0-%d2
	
	|SET_VEC3_ROT %a0,%a1

	|load the first 6 matrix elements into registers d3-d7,a4
	movem.w (%a5)+,%d3-%d7/%a4
	
	|multiply!
	muls.w %d0,%d3
	muls.w %d1,%d4
	muls.w %d2,%d5
	muls.w %d0,%d6
	muls.w %d1,%d7
	
	|add!
	add.l %d4,%d3
	add.l %d5,%d3
	
	add.l %d7,%d6
	
	move.l %a4,%d4
	muls.w %d2,%d4
	add.l %d4,%d6
	
	
	
	|load the next 3 elements
	movem.w (%a5)+,%d4-%d5/%d7
	
	muls.w %d0,%d4
	muls.w %d1,%d5
	muls.w %d2,%d7
	
	add.l %d4,%d7
	add.l %d5,%d7
	
|	swap %d7
|	move.w %d7,%d3
	asl.l #1,%d3
	asl.l #1,%d6
	
	asl.l #1,%d7
	
	|exg %d7,%d4
	
	|move.l %d3,(%a3)+
	
	|swap %d4
	|move.w %d4,(%a3)
	
	
	|addq #8,%a3
	
	movem.l %d3/%d6/%d7,(%a3)
	
	movem.l (%sp)+,%d1-%d7
	
	rts
	