# This file is part of X3D.
#
# X3D is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# X3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with X3D. If not, see <http:#www.gnu.org/licenses/>.

    .cpu arm926ej-s
	.fpu softvfp

.global draw_span_solid
.global draw_surface_span


# Calculates the reciprocal of r0
.macro fastrecip
    push	{r1, r2, r3, r4, r5}
    clz	r1, r0
    ldr	r3, fastrecip_constant
    sub	r1, r1, #16
    lsl	r2, r0, r1
    sub	r3, r3, r2, lsl #1
    smull	r4, r5, r2, r3
    rsb	r0, r1, #16
    lsr	r1, r4, #16
    orr	r1, r1, r5, lsl #16
    rsb	r1, r1, #65536
    smull	r4, r5, r1, r3
    lsr	r1, r4, #16
    orr	r1, r1, r5, lsl #16
    add	r3, r3, r1
    smull	r4, r5, r2, r3
    lsr	r1, r4, #16
    orr	r1, r1, r5, lsl #16
    rsb	r1, r1, #65536
    smull	r4, r5, r1, r3
    lsr	r1, r4, #16
    orr	r1, r1, r5, lsl #16
    add	r3, r3, r1
    smull	r4, r5, r2, r3
    lsr	r2, r4, #16
    orr	r2, r2, r5, lsl #16
    rsb	r2, r2, #65536
    smull	r4, r5, r2, r3
    lsr	r2, r4, #16
    orr	r2, r2, r5, lsl #16
    add	r3, r3, r2
    lsr	r0, r3, r0
    pop	{r1, r2, r3, r4, r5}
.endm
    
fastrecip_constant:
    .word 185042

# typedef struct X_AE_Span
# {
#     short x1;
#     short x2;
#     short y;
# } X_AE_Span;


# Draws a solid span
#   r0 - span struct
#   r1 - color
#   r2 - screen
draw_span_solid:
    push { r3, r4 }
    
    mov r3, #320            @ screenW = r3
    ldrsh r4, [r0, #4]      @ y = r4
    mla r2, r4, r3, r2      @ scanline = screen + y * screenW
    
    ldrsh r3, [r0]          @ left = r3    
    ldrsh r4, [r0, #2]      @ right = r4
    
    # We ignore spans of length zero, so return if left == right
    cmp r3, r4
    popeq { r3, r4 }
    bxeq lr
    
    # Calculate the end of the span
    add r4, r2, r4          @ end = scanline + right -> r4
    
    # Advance to start of span
    add r2, r2, r3          @ scanline += left -> r2
    
    
draw_span_solid_loop:
    strb r1, [r2], #1
    cmp r2, r4
    bne draw_span_solid_loop
    
    pop { r3, r4 }
    
    bx lr

.macro load_texel surface, surfaceW, u, v, du, dv, dest, scratch
    mov \scratch, \v, lsr #16
    mla \scratch, \surfaceW, \scratch, \surface
    add \v, \v, \dv
    ldrb \scratch, [\scratch, \u, lsr #16]
    add \u, \u, \du
    orr \dest, \scratch, \dest, lsl #8
.endm

.macro draw_aligned_span_16 scanline, surface, surfaceW, u, v, du, dv, pixelGroup, scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    str \pixelGroup,[\scanline], #-4
    
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    str \pixelGroup,[\scanline], #-4
    
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    str \pixelGroup,[\scanline], #-4
    
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    load_texel \surface, \surfaceW, \u, \v, \du, \dv, \pixelGroup, \scratch
    str \pixelGroup,[\scanline], #-4
.endm

.macro calculate_initial_inv_z x, y, invZStepX, invZStepY, invZOrigin, dest, invZDest, scratch
    # Calculate 1/z
    mov \scratch, r0
    mul \invZDest, \x, \invZStepX
    mla \invZDest, \y, \invZStepY, \invZDest
    add \invZDest, \invZDest, \invZOrigin
    mov r0, \invZDest, lsr #10
    
    # Calculate z (1/1/z)
    fastrecip
    mov \dest, r0
    mov r0, \scratch
.endm

.macro calculate_initial_u x, y, uStepX, uStepY, uOrigin, z, dest, uDivZDest
    # Calculate u/z
    mul \uDivZDest, \x, \uStepX
    mla \uDivZDest, \y, \uStepY, \uDivZDest
    add \uDivZDest, \uDivZDest, \uOrigin
    
    # Calculate u (u/z * z)
    mul \dest, \z, \uDivZDest
.endm

.macro calculate_initial_v x, y, vStepX, vStepY, vOrigin, z, dest, vDivZDest
    # Calculate v/z
    mul \vDivZDest, \x, \vStepX
    mla \vDivZDest, \y, \vStepY, \vDivZDest
    add \vDivZDest, \vDivZDest, \vOrigin
    
    # Calculate v (v/z * z)
    mul \dest, \z, \vDivZDest
.endm

.macro advance_inv_by_16 invZ, uDivZ, vDivZ, scratch
    ldr \scratch, [r0]      @ Load invZStepXNeg
    add \invZ, \invZ, \scratch, lsl #4
    
    ldr \scratch, [r0, #4]   @ Load uStepXNeg
    add \uDivZ, \uDivZ, \scratch, lsl #4
    
    ldr \scratch, [r0, #8]  @ Load vStepXNeg
    add \vDivZ, \vDivZ, \scratch, lsl #4
.endm

.macro advance_inv_by_count invZ, uDivZ, vDivZ, count, scratch
    ldr \scratch, [r0]      @ Load invZStepXNeg
    mla \invZ, \count, \scratch, \invZ

    ldr \scratch, [r0, #4]   @ Load uStepXNeg
    mla \uDivZ, \count, \scratch, \uDivZ

    ldr \scratch, [r0, #8]  @ Load vStepXNeg
    mla \vDivZ, \count, \scratch, \vDivZ
.endm

.macro adjust_and_clamp_u u, scratch
    ldr \scratch, [r0, #20]         @ scratch = uAdjustment
    
    adds \u, \u, \scratch           @ u += uAdjustment
    movmi \u, #32                   @ if(u < 0) u = roundOffGuard
    ldr \scratch, [r0, #28]         @ scratch = surfaceW
    cmp \u, \scratch
    movgt \u, \scratch              @ if(u > surfaceW) u = surfaceW
.endm

.macro adjust_and_clamp_v v, scratch
    ldr \scratch, [r0, #24]         @ scratch = vAdjustment
    
    adds \v, \v, \scratch           @ v += vAdjustment
    movmi \v, #32                   @ if(v < 0) v = roundOffGuard
    ldr \scratch, [r0, #32]         @ scratch = surfaceH
    cmp \v, \scratch
    movgt \v, \scratch              @ if(v > surfaceH) v = surfaceH
.endm
    
.macro adjust_and_clamp_uv u, v, scratch
    adjust_and_clamp_u \u, \scratch
    adjust_and_clamp_v \v, \scratch
.endm
    
# r0 -> context
# r1 -> span
draw_surface_span:
    push { r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr }
    # Load left, right, and y
    ldm r1, { r2, r3, r4 }
    
    # left = r2
    # right = r3
    # y = r4
    
    # Load 1/z variables and u variables
    ldm r0!, { r5, r6, r7, r8, r9, r10 }
    
    calculate_initial_inv_z r3, r4, r5, r6, r7, r11, r12, r14   @ Calculate z (r11) and invZ (r12) at right end of span
    
    calculate_initial_u r3, r4, r8, r9, r10, r11, r5, r6        @ Calculate u (r5) and uDivZ (r6) at right end of span
    
    ldm r0!, { r9, r10, r14 }                                   @ Load v variables
    calculate_initial_v r3, r4, r9, r10, r14, r11, r7, r8       @ Calculate v (r7) and vDivZ (r8) at right end of span
    
    adjust_and_clamp_uv r5, r7, r9
  
    # Calculate start and end address of span
    ldr r9, [r0, #12]
    mov r10, #320
    mla r9, r4, r10, r9     @ scanline (r9) = y * 320 + screenAddress
    
    add r2, r2, r9          @ r2 = span start address
    add r3, r3, r9          @ r3 = span end address
    
    ldr r1, [r0, #36]       @ r1 = surfaceTexels
    ldr r10, [r0, #16]      @ r10 = surfaceW
    
    and r4, r3, #3          @ Calculate pixels until we reach a multiple of 4 going backwards (r4)
    
    # Make sure after getting to a multiple of 4 we have at least one group of 16
    sub r9, r3, r2      @ r9 = length of scanline
    sub r9, r9, r4      @ r9 = lengthOfScanline - pixelsUntilMultipleOfFour
    cmp r9, #16
    blt draw_span_using_shorts

    cmp r4, #0
    subeq r3, #4        @ Need to start at the preceding word
    beq draw_span_using_ints
    
    add r4, r4, #16     @ r4 = count
    advance_inv_by_count r12, r6, r8, r4, r14
    
    ldr r14, [r0, #40]  @ r14 = recipTab
    ldr r11, [r14, r4, lsl #2]   @ r11 = recipTab[r4]
    
    mov r14, r0     @ Save r0
    mov r0, r12, lsr #10
    fastrecip
    
    mul r4, r6, r0      @ r4 = (u / z) * z
    mul r9, r8, r0      @ r9 = (v / z) * z
    
    mov r0, r14     @ Restore r0
    
    adjust_and_clamp_uv r4, r9, r14
    
    sub r4, r4, r5
    sub r9, r9, r7
    
    smull r4, r14, r11, r4      @ Calculate du (r4)
    asr r4, #16
    orr r4, r4, r14, lsl #16
    
    smull r9, r14, r11, r9      @ Calculate du (r9)
    asr r9, #16
    orr r9, r9, r14, lsl #16
    
    # Draw individual pixels until we reach a multiple of 4
draw_until_multiple_of_4:
    load_texel r1, r10, r5, r7, r4, r9, r11, r14
    strb r11, [r3], #-1
    ands r14, r3, #3
    bne draw_until_multiple_of_4
    
    sub r3, r3, #4
    b draw_span_using_ints_skip_uv_calculation 
    
draw_span_using_ints:
    # Calculate u and v 16 pixels to the left
    advance_inv_by_16 r12, r6, r8, r14
    mov r14, r0         @ Save r0
    mov r0, r12, lsr #10
    fastrecip
    
    mul r4, r6, r0      @ r4 = (u / z) * z
    mul r9, r8, r0      @ r9 = (v / z) * z
    
    mov r0, r14         @ Restore r0
    
    adjust_and_clamp_uv r4, r9, r11
    
    sub r4, r4, r5
    asr r4, #4          @ r4 = du
    
    sub r9, r9, r7
    asr r9, #4          @ r9 = dv

# u -> r5
# v -> r7
# du -> r4
# dv -> r9
# surface texels -> r1
# surfaceW -> r10
# currentPixel -> r3
# beginning of span -> r2
draw_span_using_ints_skip_uv_calculation:
    draw_aligned_span_16 r3, r1, r10, r5, r7, r4, r9, r11, r14
    sub r14, r3, #16
    cmp r14, r2
    bge draw_span_using_ints
    
    add r3, #4
    
draw_span_using_shorts:
    subs r4, r3, r2
    beq done

    advance_inv_by_count r12, r6, r8, r4, r14
    
    ldr r14, [r0, #40]  @ r14 = recipTab
    ldr r11, [r14, r4, lsl #2]   @ r11 = recipTab[r4]
    
    mov r14, r0     @ Save r0
    mov r0, r12, lsr #10
    fastrecip
    
    mul r4, r6, r0      @ r4 = (u / z) * z
    mul r9, r8, r0      @ r9 = (v / z) * z
    
    mov r0, r14     @ Restore r0
    
    adjust_and_clamp_uv r4, r9, r14
    
    sub r4, r4, r5
    sub r9, r9, r7
    
    smull r4, r14, r11, r4      @ Calculate du (r4)
    asr r4, #16
    orr r4, r4, r14, lsl #16
    
    smull r9, r14, r11, r9      @ Calculate du (r9)
    asr r9, #16
    
write_texel_loop:
    mov r11, #14
    #load_texel r1, r10, r5, r7, r4, r9, r11, r14
    strb r11, [r3], #-1
    cmp r2, r3
    bne write_texel_loop
    
done:
    pop { r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr }
    bx lr

