	.file	"test.c"
	.option nopic
	.attribute arch, "rv32i2p1_m2p0_a2p1_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata
	.align	2
.LC0:
	.string	" in funct"

funct:
	addi	sp,sp,-16
	sw	ra,12(sp)
	
	addi	s0,sp,16
	lui	a5,%hi(.LC0)
	addi	a0,a5,%lo(.LC0)
	call	printf
	
	lw	ra,12(sp)

	addi	sp,sp,16
	jr	ra

	.section	.rodata
	.align	2
.LC1:
	.string	"string in main"


	.globl	main
	.type	main, @function
main:

	call	funct
	lui	a5,%hi(.LC1)
	addi	a0,a5,%lo(.LC1)
	call	puts



	addi	sp,sp,16
	jr	ra

