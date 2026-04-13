  .data
  .globl x
x:
  .word 10
  .word 20

  .text
  .globl main
main:
  addi sp, sp, -48
  addi t0, sp, 0
  slli t1, x0, 2
  add t0, t0, t1
  sw t0, 20(sp)
  li t0, 1
  lw t1, 20(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 1
  slli t1, t1, 2
  add t0, t0, t1
  sw t0, 24(sp)
  li t0, 2
  lw t1, 24(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 2
  slli t1, t1, 2
  add t0, t0, t1
  sw t0, 28(sp)
  li t0, 3
  lw t1, 28(sp)
  sw t0, 0(t1)
  addi t0, sp, 0
  li t1, 3
  slli t1, t1, 2
  add t0, t0, t1
  sw t0, 32(sp)
  lw t1, 32(sp)
  sw x0, 0(t1)
  addi t0, sp, 0
  li t1, 4
  slli t1, t1, 2
  add t0, t0, t1
  sw t0, 36(sp)
  lw t1, 36(sp)
  sw x0, 0(t1)
  addi t0, sp, 0
  li t1, 2
  slli t1, t1, 2
  add t0, t0, t1
  sw t0, 40(sp)
  lw t1, 40(sp)
  lw t0, 0(t1)
  sw t0, 44(sp)
  lw a0, 44(sp)
  addi sp, sp, 48
  ret

