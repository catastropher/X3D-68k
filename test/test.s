	.file	"test.c"
#NO_APP
	.text
tigcc_compiled.:
	.text
#APP
	.xdef __ref_all___startup_code
	.text
	.xdef __ref_all___MIN_AMS_required
	.text
	.xdef __ref_all___MIN_AMS_1_01
	.text
	.set _A_LINE,0xA000
	.text
	.xdef __ref_all___kernel_format_data_var
	.text
	.xdef _tigcc_native
	.text
	.xdef __ref_all___nostub
	.text
	.xdef __ref_all___save_screen
	.text
	.xdef __ref_all___kernel_format_bss
	.text
	.xdef __ref_all___kernel_format_rom_calls
	.text
	.set MT_TEXT,0x8000
	.text
	.set MT_XREF,0x9000
	.text
	.set MT_ICON,0xA000
	.text
	.set MT_CASCADE,0x4000
	.text
.LC0:
	.ascii "%ld\12\0"
#NO_APP
	.text
	.even
	.globl	__main
__main:
	link.w %fp,#-16
	jbsr clrscr
	move.w #5,-10(%fp)
	move.w #10,-8(%fp)
	move.w #15,-6(%fp)
	move.w #10,-16(%fp)
	move.w #20,-14(%fp)
	move.w #30,-12(%fp)
	lea (-16,%fp),%a0
	move.l %a0,-(%sp)
	lea (-10,%fp),%a0
	move.l %a0,-(%sp)
	jbsr x3d_vex3d_int16_dot
	addq.l #8,%sp
	move.l %d0,-4(%fp)
	move.l -4(%fp),-(%sp)
	pea .LC0
	jbsr printf
	addq.l #8,%sp
#APP
	.xdef __ref_all___set_file_in_use_bit
#NO_APP
	move.w #200,%a0
	move.l (%a0),%d0
	move.l %d0,%a0
	lea (324,%a0),%a0
	move.l (%a0),%d0
	move.l %d0,%a0
	jbsr (%a0)
	unlk %fp
	rts
.comm _ti92plus,2
