
"Premature optimization is the root of all evil"
（过早优化是万恶之源）

1. 查看每条指令类型 ，计算分配的 sp 内存大小S
2. S 对齐 16 S'
3. 函数入口要 addi
4. 对于在指令中用到的其他指令的返回值, 比如 add %1, %2 中的 %1 和 %2, 用 lw 指令从栈帧中读数据到临时寄存器中, 然后再计算结果 
5. 对于所有存在返回值的指令, 比如 load 和 add, 计算出指令的返回值后, 用 sw 指令把返回值存入栈帧.
6. 函数返回前, 即 ret 指令之前, 你需要生成复原栈指针的指令, 将栈指针加上 S'这个过程叫做函数的 epilogue.
  .text
  .globl main
main:
  # 函数的 prologue
  addi sp, sp, -16

  # store 10, @x
  li t0, 10 
  sw t0, 0(sp) //把t0 寄存器的值存入 sp+0;

  # %0 = load @x
  lw t0, 0(sp) //先读取 sp+0 的值 放到 t0
  sw t0, 4(sp) //把t0 寄存器       sp+4

  # %1 = add %0, 1
  lw t0, 4(sp) //
  li t1, 1
  add t0, t0, t1
  sw t0, 8(sp)

  # store %1, @x
  lw t0, 8(sp)
  sw t0, 0(sp)

  # %2 = load @x
  lw t0, 0(sp)
  sw t0, 12(sp)

  # ret %2, 以及函数的 epilogue
  lw a0, 12(sp)
  addi sp, sp, 16
  ret
