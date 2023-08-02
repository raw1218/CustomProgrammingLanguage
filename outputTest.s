	.file	"test.c"
	.option nopic
	.attribute arch, "rv32i2p1_m2p0_a2p1_c2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata
	.align	2

	.align	1
	.globl	main
	
main:
  addi sp, x0, 1000
  addi sp, x0, 1000
  addi s0, x0, 1000
  ADD fp, sp, x0