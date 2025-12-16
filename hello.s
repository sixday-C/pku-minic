  .text
  .globl main
main:
  addi sp, sp, -32
entry:
  li t0, 5
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 10
  add t0, t0, t1
  sw t0, 12(sp)
  lw t0, 12(sp)
  sw t0, 4(sp)
  lw t0, 4(sp)
  sw t0, 16(sp)
  lw a0, 16(sp)
  addi sp, sp, 32
  ret
