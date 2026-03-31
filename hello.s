  .text
  .globl half
half:
  addi sp, sp, -16
  sw a0, 0(sp)
  lw t0, 0(sp)
  sw t0, 4(sp)
  lw t0, 4(sp)
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 2
  div t0, t0, t1
  sw t0, 12(sp)
  lw a0, 12(sp)
  addi sp, sp, 16
  ret

  .text
  .globl f
f:
  ret

  .text
  .globl main
main:
  addi sp, sp, -16
  sw ra, 12(sp)
  call f
  li a0, 10
  call half
  sw a0, 0(sp)
  lw a0, 0(sp)
  lw ra, 12(sp)
  addi sp, sp, 16
  ret

