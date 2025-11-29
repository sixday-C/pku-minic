  .text
  .globl main
main:
  li   t0, 6
  seqz t0, t0
  sub  t1, x0, t0
  sub  t2, x0, t1
  mv   a0, t2
  ret
