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

.global draw_span_solid
.global fast_div

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

    
# r0 -> scanline
# r1 -> surface address
# r2 -> surface width
# r3 -> u
# r4 -> v
# r5 -> du
# r6 -> dv
draw_aligned_span_16:
    # r11 is scratch
    mov r11, r4, lsr #16
    mla r11, r2, r11, r1            @ texelAddr = surfaceW * (v >> 16) + surfaceAddr
    ldrb r11, [r11, r3, lsr #16]    @ pixel = *(texelAddr + u >> 16)
    orr r7, r11, r7, lsl #8         @ a = (a << 8) | pixel
    add r3, r3, r5                  @ u += du
    add r4, r4, r6                  @ v += dv
    
# r0 -> span pointer
# r1 -> surface address
# r3 -> surface w
draw_span_asm:
    
    
    
    
    
