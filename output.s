.option nopic
.attribute arch, "rv32i2p1_m2p0_a2p1_c2p0"
.attribute unaligned_access, 0
.attribute stack_align, 16
.text
.section .rodata
  .align 4
.LCD:
  .string "%d"
  .align 4
.LCF:
  .string "%d."
  .align 4
.LCBT:
  .string "True"
  .align 4
.LCBF:
  .string "False"
  .align 4
.LCEmpty:
  .string ""
.align 1
Addition_2Word:
  ADD t0, a1, a3
  BLTU t0, a1, overflow_detected0
  BLTU t0, a3, overflow_detected0
  JAL x0, no_overflow_detected0
overflow_detected0:
  ADDI a0, a0, 1
no_overflow_detected0:
  ADD a0, a0, a2
  ADD a1, x0, t0
  JALR x0, ra, 0
Multiplication_Unsigned_1Word:
  ADDI sp, sp, -24
  SW s0, 0(sp)
  SW s1, 4(sp)
  SW s2, 8(sp)
  SW s3, 12(sp)
  SW s4, 16(sp)
  SW s5, 20(sp)
  ADD s0, x0, x0
  ADD s1, a0, x0
  ADD s2, x0, x0
  ADD s3, a1, x0
  ADD s4, x0, x0
  ADD s5, x0, x0
Multiplication_Unsigned_1Word_loop:
  BEQ s3, x0, Multiplication_Unsigned_1Word_end
  ANDI t0, s3, 1
  BEQ t0, x0, Multiplication_Unsigned_1Word_loop_end
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, s0, zero
  ADD a1, s1, zero
  ADD a2, s4, zero
  ADD a3, s5, zero
  JAL ra, Addition_2Word
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD s4, a0, zero
  ADD s5, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
Multiplication_Unsigned_1Word_loop_end:
  SRLI s3, s3, 1
  SLLI s0, s0, 1
  BGE s1, x0, skip_next1
 ADDI s0, s0, 1
skip_next1:
  SLLI s1, s1, 1
  JAL x0, Multiplication_Unsigned_1Word_loop
Multiplication_Unsigned_1Word_end:
  ADD a0, s4, x0
  ADD a1, s5, x0
  LW s0, 0(sp)
  LW s1, 4(sp)
  LW s2, 8(sp)
  LW s3, 12(sp)
  LW s4, 16(sp)
  LW s5, 20(sp)
  ADDI sp, sp, 24
  JALR x0, ra, 0
Multiplication_Unsigned_FixedPoint:
  ADDI sp, sp, -40
  SW s0, 0(sp)
  SW s1, 4(sp)
  SW s2, 8(sp)
  SW s3, 12(sp)
  SW s4, 16(sp)
  SW s5, 20(sp)
  SW s6, 24(sp)
  SW s7, 28(sp)
  SW s8, 32(sp)
  SW s9, 36(sp)
  ADD s0, x0, x0
  ADD s1, x0, x0
  ADDI sp, sp, -12
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW ra, 8(sp)
  ADD a0, a0, x0
  ADD a1, a2, x0
  JAL ra, Multiplication_Unsigned_1Word
  ADD s2, a0, x0
  ADD s3, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW ra, 8(sp)
  ADDI sp, sp, 12
  ADDI sp, sp, -12
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW ra, 8(sp)
  ADD a0, a0, x0
  ADD a1, a3, x0
  JAL ra, Multiplication_Unsigned_1Word
  ADD s4, a0, x0
  ADD s5, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW ra, 8(sp)
  ADDI sp, sp, 12
  ADDI sp, sp, -12
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW ra, 8(sp)
  ADD a0, a1, x0
  ADD a1, a2, x0
  JAL ra, Multiplication_Unsigned_1Word
  ADD s6, a0, x0
  ADD s7, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW ra, 8(sp)
  ADDI sp, sp, 12
  ADDI sp, sp, -12
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW ra, 8(sp)
  ADD a0, a1, x0
  ADD a1, a3, x0
  JAL ra, Multiplication_Unsigned_1Word
  ADD s8, a0, x0
  ADD s9, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW ra, 8(sp)
  ADDI sp, sp, 12
  ADDI sp, sp, -20
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW ra, 16(sp)
  ADD a0, s0, x0
  ADD a1, s1, x0
  ADD a2, s3, x0
  ADD a3, x0, x0
  JAL ra, Addition_2Word
  ADD s0, a0, x0
  ADD s1, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 16(sp)
  ADDI sp, sp, 20
  ADDI sp, sp, -20
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW ra, 16(sp)
  ADD a0, s0, x0
  ADD a1, s1, x0
  ADD a2, s4, x0
  ADD a3, s5, x0
  JAL ra, Addition_2Word
  ADD s0, a0, x0
  ADD s1, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 16(sp)
  ADDI sp, sp, 20
  ADDI sp, sp, -20
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW ra, 16(sp)
  ADD a0, s0, x0
  ADD a1, s1, x0
  ADD a2, s6, x0
  ADD a3, s7, x0
  JAL ra, Addition_2Word
  ADD s0, a0, x0
  ADD s1, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 16(sp)
  ADDI sp, sp, 20
  ADDI sp, sp, -20
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW ra, 16(sp)
  ADD a0, s0, x0
  ADD a1, s1, x0
  ADD a2, x0, x0
  ADD a3, s8, x0
  JAL ra, Addition_2Word
  ADD s0, a0, x0
  ADD s1, a1, x0
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 16(sp)
  ADDI sp, sp, 20
  ADD a0, s0, x0
  ADD a1, s1, x0
  LW s0, 0(sp)
  LW s1, 4(sp)
  LW s2, 8(sp)
  LW s3, 12(sp)
  LW s4, 16(sp)
  LW s5, 20(sp)
  LW s6, 24(sp)
  LW s7, 28(sp)
  LW s8, 32(sp)
  LW s9, 36(sp)
  ADDI sp, sp, 40
  JALR x0, ra, 0
Multiplication_Signed_1Word:
  ADD t0, a0, x0
  ADD t1, a1, x0
  BGE t0, x0, left_skip_next2
  XORI t0, t0, -1
  ADDI t0, t0, 1
left_skip_next2:
  BGE t1, x0, right_skip_next2
  XORI t1, t1, -1
  ADDI t1, t1, 1
right_skip_next2:
  ADDI sp, sp, -40
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW t0, 8(sp)
  SW t1, 12(sp)
  SW t2, 16(sp)
  SW t3, 20(sp)
  SW t4, 24(sp)
  SW t5, 28(sp)
  SW t6, 32(sp)
  SW ra, 36(sp)
  ADD a0, t0, zero
  ADD a1, t1, zero
  JAL ra, Multiplication_Unsigned_1Word
  LW t0, 8(sp)
  LW t1, 12(sp)
  LW t2, 16(sp)
  LW t3, 20(sp)
  LW t4, 24(sp)
  LW t5, 28(sp)
  LW t6, 32(sp)
  ADD t0, a0, zero
  ADD t1, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW ra, 36(sp)
  ADDI sp, sp, 40
  XOR t2, a0, a1
  BGE t2, x0, different_signs_skip_next2
  XORI t0, t0, -1
  XORI t1, t1, -1
  ADDI t2, x0, 1
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, t0, zero
  ADD a1, t1, zero
  ADD a2, x0, zero
  ADD a3, t2, zero
  JAL ra, Addition_2Word
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD t0, a0, zero
  ADD t1, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
 different_signs_skip_next2:
  ADD a0, t0, x0
  ADD a1, t1, x0
  JALR x0, ra, 0
Multiplication_Signed_FixedPoint:
  ADD t0, a0, x0
  ADD t1, a1, x0
  ADD t2, a2, x0
  ADD t3, a3, x0
  BGE t0, x0, left_skip_next3
  XORI t0, t0, -1
  XORI t1, t1, -1
  ADDI t5, x0, 1
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, t0, zero
  ADD a1, t1, zero
  ADD a2, x0, zero
  ADD a3, t5, zero
  JAL ra, Addition_2Word
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD t0, a0, zero
  ADD t1, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
left_skip_next3:
  BGE t2, x0, right_skip_next3
  XORI t2, t0, -1
  XORI t3, t1, -1
  ADDI t5, x0, 1
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, t2, zero
  ADD a1, t3, zero
  ADD a2, x0, zero
  ADD a3, t5, zero
  JAL ra, Addition_2Word
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD t2, a0, zero
  ADD t3, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
right_skip_next3:
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, t0, zero
  ADD a1, t1, zero
  ADD a2, t2, zero
  ADD a3, t3, zero
  JAL ra, Multiplication_Unsigned_FixedPoint
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD t0, a0, zero
  ADD t1, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
  XOR t3, a0, a2
  BGE t3, x0, different_signs_skip_next3
  XORI t0, t0, -1
  XORI t1, t1, -1
  ADDI t3, x0, 1
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, t0, zero
  ADD a1, t1, zero
  ADD a2, x0, zero
  ADD a3, t3, zero
  JAL ra, Addition_2Word
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD t0, a0, zero
  ADD t1, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
different_signs_skip_next3:
  ADD a0, t0, x0
  ADD a1, t1, x0
  JALR x0, ra, 0
Subtraction_2Word:
  SUB t0, a1, a3
  BGEU a1, t0, no_underflow_detected4
  JAL x0, underflow_detected4
underflow_detected4:
  ADDI a0, a0, -1
no_underflow_detected4:
  SUB a0, a0, a2
  ADD a1, x0, t0
  JALR x0, ra, 0
Division_Unsigned_2Word:
  ADDI sp, sp, -48
  SW s0, 0(sp)
  SW s1, 4(sp)
  SW s2, 8(sp)
  SW s3, 12(sp)
  SW s4, 16(sp)
  SW s5, 20(sp)
  SW s6, 24(sp)
  SW s7, 28(sp)
  SW s8, 32(sp)
  SW s9, 36(sp)
  SW s10, 40(sp)
  SW s11, 44(sp)
  ADD s0, x0, x0
  ADD s1, x0, x0
  ADD s2, x0, x0
  ADD s3, x0, x0
  ADD s4, x0, x0
  ADD s5, x0, x0
  ADD s6, a0, x0
  ADD s7, a1, x0
  ADDI s8, x0, 1
  ADD s9, x0, x0
  ADD s10, a2, x0
  ADD s11, a3, x0
Division_Unsigned_2Word_Shift_Loop:
  BLTU s6, s10, Action5
  BLTU s10, s6, Division_Unsigned_2Word_Shift_Loop_End
  BGEU s7, s11, Division_Unsigned_2Word_Shift_Loop_End
Action5:
   SLT t0, s7, x0
  SLLI s6, s6, 1
  SLLI s7, s7, 1
  ADD s6, s6, t0
  ADDI s9, s9, 1
  JAL x0, Division_Unsigned_2Word_Shift_Loop
Division_Unsigned_2Word_Shift_Loop_End:
Division_Unsigned_2Word_Loop:
  ADDI t0, x0, 129
  BEQ s8, t0, Division_Unsigned_2Word_Loop_End
  BNE s4, x0, loop_skip_next5
  BNE s5, x0, loop_skip_next5
  BNE s6, x0, loop_skip_next5
  BEQ s7, x0, Division_Unsigned_2Word_Loop_End
loop_skip_next5:
  SLT t0, s6, x0
  SLT t1, s5, x0
  SLLI s5, s5, 1
  SLLI s4, s4, 1
  ADD s4, s4, t1
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, s4, zero
  ADD a1, s5, zero
  ADD a2, x0, zero
  ADD a3, t0, zero
  JAL ra, Addition_2Word
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD s4, a0, zero
  ADD s5, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
   SLT t0, s7, x0
  SLLI s6, s6, 1
  SLLI s7, s7, 1
  ADD s6, s6, t0
  BLTU s4, a2, Division_Unsigned_2Word_Loop_Branch_False
  BLTU a2, s4, Division_Unsigned_2Word_Loop_Branch_True
  BLTU s5, a3, Division_Unsigned_2Word_Loop_Branch_False
Division_Unsigned_2Word_Loop_Branch_True:
  ADDI sp, sp, -48
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW a2, 8(sp)
  SW a3, 12(sp)
  SW t0, 16(sp)
  SW t1, 20(sp)
  SW t2, 24(sp)
  SW t3, 28(sp)
  SW t4, 32(sp)
  SW t5, 36(sp)
  SW t6, 40(sp)
  SW ra, 44(sp)
  ADD a0, s4, zero
  ADD a1, s5, zero
  ADD a2, a2, zero
  ADD a3, a3, zero
  JAL ra, Subtraction_2Word
  LW t0, 16(sp)
  LW t1, 20(sp)
  LW t2, 24(sp)
  LW t3, 28(sp)
  LW t4, 32(sp)
  LW t5, 36(sp)
  LW t6, 40(sp)
  ADD s4, a0, zero
  ADD s5, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW a2, 8(sp)
  LW a3, 12(sp)
  LW ra, 44(sp)
  ADDI sp, sp, 48
  ADDI t0, x0, 97
  BGEU s8, t0, Iteration_GT_965
  ADDI t0, x0, 65
  BGEU s8, t0, Iteration_GT_645
  ADDI t0, x0, 33
  BGEU s8, t0, Iteration_GT_325
  JAL x0, Iteration_LTE_325
Iteration_GT_965:
  ADDI t1, x0, 128
  SUB t1, t1, s8
  ADDI t2, x0, 1
  SLL t1, t2, t1
  OR s3, s3, t1
  JAL x0, Iteration_Branch_End5
Iteration_GT_645:
  ADDI t1, x0, 96
  SUB t1, t1, s8
  ADDI t2, x0, 1
  SLL t1, t2, t1
  OR s2, s2, t1
  JAL x0, Iteration_Branch_End5
Iteration_GT_325:
  ADDI t1, x0, 64
  SUB t1, t1, s8
  ADDI t2, x0, 1
  SLL t1, t2, t1
  OR s1, s1, t1
  JAL x0, Iteration_Branch_End5
Iteration_LTE_325:
  ADDI t1, x0, 32
  SUB t1, t1, s8
  ADDI t2, x0, 1
  SLL t1, t2, t0
  OR s0, s0, t1
Iteration_Branch_End5:
  JAL x0, Division_Unsigned_2Word_Loop_Branch_End
Division_Unsigned_2Word_Loop_Branch_False:
Division_Unsigned_2Word_Loop_Branch_End:
  ADDI s8, s8, 1
  JAL x0, Division_Unsigned_2Word_Loop
Division_Unsigned_2Word_Loop_End:
Shift_Right_Loop5:
  BGE x0, s9, Shift_Right_Loop_End5
  ANDI t0, s0, 1
  ANDI t1, s1, 1
  ANDI t2, s2, 1
  SLLI t0, t0, 31
  SLLI t1, t1, 31
  SLLI t2, t2, 31
  SRLI s0, s0, 1
  SRLI s1, s1, 1
  SRLI s2, s2, 1
  SRLI s3, s3, 1
  OR s1, s1, t0
  OR s2, s2, t1
  OR s3, s3, t2
  ADDI s9, s9, -1
  JAL x0, Shift_Right_Loop5
Shift_Right_Loop_End5:
  ADD a0, s0, x0
  ADD a1, s1, x0
  ADD a2, s2, x0
  ADD a3, s3, x0
  LW s0, 0(sp)
  LW s1, 4(sp)
  LW s2, 8(sp)
  LW s3, 12(sp)
  LW s4, 16(sp)
  LW s5, 20(sp)
  LW s6, 24(sp)
  LW s7, 28(sp)
  LW s8, 32(sp)
  LW s9, 36(sp)
  LW s10, 40(sp)
  LW s11, 44(sp)
  ADDI sp, sp, 48
  JALR x0, ra, 0
Print:
  addi sp, sp, -16
  sw ra, 12(sp)
  sw s0, 8(sp)
  call printf
  lw ra, 12(sp)
  lw s0, 8(sp)
  addi sp, sp, 16
  JALR x0, ra, 0
OrFunction:
  ADD t1, a0, a1
  SLTU a0, x0, t1
  JALR x0, ra, 0
AndFunction:
  ADD t1, a0, a1
  ADDI t1, t1, -1
  SLT a0, x0, t1
  JALR x0, ra, 0
NotFunction:
  SLTIU a0, a0, 1
  JALR x0, ra, 0
LT2Word:
  BLT a0, a2, EndTrue6
  BLT a2, a0, EndFalse6
  SLT t0, a0, x0
  XORI t1, t0, 1
  BLTU a1, a3, LowerLT6
  BLTU a3, a1, LowerGT6
  ADDI a0, x0, 0
  JALR x0, ra, 0
EndTrue6:
  ADDI a0, x0, 1
  JALR x0, ra, 0
EndFalse6:
  ADDI a0, x0, 0
  JALR x0, ra, 0
LowerLT6:
  ADD a0, t0, x0
  JALR x0, ra, 0
LowerGT6:
  ADD a0, t1, x0
  JALR x0, ra, 0
LTE2Word:
  BLT a0, a2, EndTrue7
  BLT a2, a0, EndFalse7
  SLT t0, a0, x0
  XORI t1, t0, 1
  BLTU a1, a3, LowerLT7
  BLTU a3, a1, LowerGT7
  ADDI a0, x0, 1
  JALR x0, ra, 0
EndTrue7:
  ADDI a0, x0, 1
  JALR x0, ra, 0
EndFalse7:
  ADDI a0, x0, 0
  JALR x0, ra, 0
LowerLT7:
  ADD a0, t0, x0
  JALR x0, ra, 0
LowerGT7:
  ADD a0, t1, x0
  JALR x0, ra, 0
ConvertFractionToDecimalInteger:
  add t0, a0, x0
  srai t1, t0, 16
  andi t1, t1, 1
  li t3,  0xFFFF0000
  and t0, t0, t3
  beq t1, x0, skip_rounding8
  li t3, 0x00010000
  add t0, t0, t3
  skip_rounding8:
  Add a0, t0, x0
  addi	sp,sp,-16
  sw	ra,12(sp)
  sw	s0,8(sp)
  sw a1, 0(sp)
  sw a0, 4(sp)
  addi	s0,sp,16
  lui	a0,%hi(.LCEmpty)
  addi	a0,a0,%lo(.LCEmpty)
  call	printf
  lw	ra,12(sp)
  lw	s0,8(sp)
  lw  a1, 0(sp)
  lw a0, 4(sp)
  addi	sp,sp,16
  ADD t0, x0, x0
  ADD t2, a0, x0
  ADDI t3, x0, 10
  ADDI t5, x0, 0
Loop8:
  ADDI t5, t5, 1
  ADDI t3, x0, 6
  BEQ t5, t3, End8
  ADDI t3, x0, 10
  BEQ t2, x0, End8
  ADDI sp, sp, -40
  SW a0, 0(sp)
  SW a1, 4(sp)
  SW t0, 8(sp)
  SW t1, 12(sp)
  SW t2, 16(sp)
  SW t3, 20(sp)
  SW t4, 24(sp)
  SW t5, 28(sp)
  SW t6, 32(sp)
  SW ra, 36(sp)
  ADD a0, t2, zero
  ADD a1, t3, zero
  JAL ra, Multiplication_Unsigned_1Word
  LW t0, 8(sp)
  LW t1, 12(sp)
  LW t2, 16(sp)
  LW t3, 20(sp)
  LW t4, 24(sp)
  LW t5, 28(sp)
  LW t6, 32(sp)
  ADD t1, a0, zero
  ADD t2, a1, zero
  LW a0, 0(sp)
  LW a1, 4(sp)
  LW ra, 36(sp)
  ADDI sp, sp, 40
  addi	sp,sp,-16
  sw	ra,12(sp)
  sw	s0,8(sp)
  sw a1, 0(sp)
  sw a0, 4(sp)
  addi	s0,sp,16
  lui	a0,%hi(.LCD)
  addi	a0,a0,%lo(.LCD)
  add a1, x0, t1
  call	printf
  lw	ra,12(sp)
  lw	s0,8(sp)
  lw  a1, 0(sp)
  lw a0, 4(sp)
  addi	sp,sp,16
  ADD t0, t0, t1
  ADD t1, x0, x0
  JAL x0, Loop8
End8:
  JALR x0, ra, 0
	.align	4
	.globl	main
main:
  ADD fp, sp, x0
  addi t0, x0, 5
  ADD t1, t0, x0
  ADDI sp, sp, -4
  SW t1, 0(sp)
jalr x0, ra, 0
