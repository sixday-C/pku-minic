  .data
  .globl x
x:
  .zero 4

  .globl y
y:
  .zero 4

  .text
  .globl t
t:
  addi sp, sp, -16
  la t0, x
  lw t0, 0(t0)
  sw t0, 0(sp)
  lw t0, 0(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 4(sp)
  lw t0, 4(sp)
  la t1, x
  sw t0, 0(t1)
  li a0, 1
  addi sp, sp, 16
  ret

  .text
  .globl f
f:
  addi sp, sp, -16
  la t0, y
  lw t0, 0(t0)
  sw t0, 0(sp)
  lw t0, 0(sp)
  li t1, 1
  add t0, t0, t1
  sw t0, 4(sp)
  lw t0, 4(sp)
  la t1, y
  sw t0, 0(t1)
  mv a0, x0
  addi sp, sp, 16
  ret

  .text
  .globl main
main:
  addi sp, sp, -544
  sw ra, 540(sp)
  sw x0, 0(sp)
  lw t0, 0(sp)
  sw t0, 4(sp)
  call f
  sw a0, 12(sp)
  lw t0, 12(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 16(sp)
  li t0, 1
  sw t0, 8(sp)
  lw t0, 16(sp)
  bnez t0, main_lor_end_1
  j main_lor_next_0
main_lor_next_0:
  call f
  sw a0, 20(sp)
  lw t0, 20(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 24(sp)
  lw t0, 24(sp)
  sw t0, 8(sp)
  j main_lor_end_1
main_lor_end_1:
  lw t0, 8(sp)
  sw t0, 28(sp)
  lw t0, 4(sp)
  lw t1, 28(sp)
  add t0, t0, t1
  sw t0, 32(sp)
  lw t0, 32(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 36(sp)
  call f
  sw a0, 44(sp)
  lw t0, 44(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 48(sp)
  li t0, 1
  sw t0, 40(sp)
  lw t0, 48(sp)
  bnez t0, main_lor_end_3
  j main_lor_next_2
main_lor_next_2:
  call t
  sw a0, 52(sp)
  lw t0, 52(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 56(sp)
  lw t0, 56(sp)
  sw t0, 40(sp)
  j main_lor_end_3
main_lor_end_3:
  lw t0, 40(sp)
  sw t0, 60(sp)
  lw t0, 36(sp)
  lw t1, 60(sp)
  add t0, t0, t1
  sw t0, 64(sp)
  lw t0, 64(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 68(sp)
  call t
  sw a0, 76(sp)
  lw t0, 76(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 80(sp)
  li t0, 1
  sw t0, 72(sp)
  lw t0, 80(sp)
  bnez t0, main_lor_end_5
  j main_lor_next_4
main_lor_next_4:
  call f
  sw a0, 84(sp)
  lw t0, 84(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 88(sp)
  lw t0, 88(sp)
  sw t0, 72(sp)
  j main_lor_end_5
main_lor_end_5:
  lw t0, 72(sp)
  sw t0, 92(sp)
  lw t0, 68(sp)
  lw t1, 92(sp)
  add t0, t0, t1
  sw t0, 96(sp)
  lw t0, 96(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 100(sp)
  call t
  sw a0, 108(sp)
  lw t0, 108(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 112(sp)
  li t0, 1
  sw t0, 104(sp)
  lw t0, 112(sp)
  bnez t0, main_lor_end_7
  j main_lor_next_6
main_lor_next_6:
  call t
  sw a0, 116(sp)
  lw t0, 116(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 120(sp)
  lw t0, 120(sp)
  sw t0, 104(sp)
  j main_lor_end_7
main_lor_end_7:
  lw t0, 104(sp)
  sw t0, 124(sp)
  lw t0, 100(sp)
  lw t1, 124(sp)
  add t0, t0, t1
  sw t0, 128(sp)
  lw t0, 128(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 132(sp)
  call f
  sw a0, 140(sp)
  lw t0, 140(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 144(sp)
  sw x0, 136(sp)
  lw t0, 144(sp)
  bnez t0, main_land_right_8
  j main_land_end_9
main_land_right_8:
  call f
  sw a0, 148(sp)
  lw t0, 148(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 152(sp)
  lw t0, 152(sp)
  sw t0, 136(sp)
  j main_land_end_9
main_land_end_9:
  lw t0, 136(sp)
  sw t0, 156(sp)
  lw t0, 132(sp)
  lw t1, 156(sp)
  add t0, t0, t1
  sw t0, 160(sp)
  lw t0, 160(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 164(sp)
  call f
  sw a0, 172(sp)
  lw t0, 172(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 176(sp)
  sw x0, 168(sp)
  lw t0, 176(sp)
  bnez t0, main_land_right_10
  j main_land_end_11
main_land_right_10:
  call t
  sw a0, 180(sp)
  lw t0, 180(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 184(sp)
  lw t0, 184(sp)
  sw t0, 168(sp)
  j main_land_end_11
main_land_end_11:
  lw t0, 168(sp)
  sw t0, 188(sp)
  lw t0, 164(sp)
  lw t1, 188(sp)
  add t0, t0, t1
  sw t0, 192(sp)
  lw t0, 192(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 196(sp)
  call t
  sw a0, 204(sp)
  lw t0, 204(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 208(sp)
  sw x0, 200(sp)
  lw t0, 208(sp)
  bnez t0, main_land_right_12
  j main_land_end_13
main_land_right_12:
  call f
  sw a0, 212(sp)
  lw t0, 212(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 216(sp)
  lw t0, 216(sp)
  sw t0, 200(sp)
  j main_land_end_13
main_land_end_13:
  lw t0, 200(sp)
  sw t0, 220(sp)
  lw t0, 196(sp)
  lw t1, 220(sp)
  add t0, t0, t1
  sw t0, 224(sp)
  lw t0, 224(sp)
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 228(sp)
  call t
  sw a0, 236(sp)
  lw t0, 236(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 240(sp)
  sw x0, 232(sp)
  lw t0, 240(sp)
  bnez t0, main_land_right_14
  j main_land_end_15
main_land_right_14:
  call t
  sw a0, 244(sp)
  lw t0, 244(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 248(sp)
  lw t0, 248(sp)
  sw t0, 232(sp)
  j main_land_end_15
main_land_end_15:
  lw t0, 232(sp)
  sw t0, 252(sp)
  lw t0, 228(sp)
  lw t1, 252(sp)
  add t0, t0, t1
  sw t0, 256(sp)
  lw t0, 256(sp)
  sw t0, 0(sp)
  call t
  sw a0, 264(sp)
  lw t0, 264(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 268(sp)
  li t0, 1
  sw t0, 260(sp)
  lw t0, 268(sp)
  bnez t0, main_lor_end_17
  j main_lor_next_16
main_lor_next_16:
  call t
  sw a0, 276(sp)
  lw t0, 276(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 280(sp)
  sw x0, 272(sp)
  lw t0, 280(sp)
  bnez t0, main_land_right_18
  j main_land_end_19
main_land_right_18:
  call t
  sw a0, 284(sp)
  lw t0, 284(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 288(sp)
  lw t0, 288(sp)
  sw t0, 272(sp)
  j main_land_end_19
main_land_end_19:
  lw t0, 272(sp)
  sw t0, 292(sp)
  lw t0, 292(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 296(sp)
  lw t0, 296(sp)
  sw t0, 260(sp)
  j main_lor_end_17
main_lor_end_17:
  lw t0, 260(sp)
  sw t0, 300(sp)
  call f
  sw a0, 308(sp)
  lw t0, 308(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 312(sp)
  li t0, 1
  sw t0, 304(sp)
  lw t0, 312(sp)
  bnez t0, main_lor_end_21
  j main_lor_next_20
main_lor_next_20:
  call t
  sw a0, 320(sp)
  lw t0, 320(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 324(sp)
  sw x0, 316(sp)
  lw t0, 324(sp)
  bnez t0, main_land_right_22
  j main_land_end_23
main_land_right_22:
  call t
  sw a0, 328(sp)
  lw t0, 328(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 332(sp)
  lw t0, 332(sp)
  sw t0, 316(sp)
  j main_land_end_23
main_land_end_23:
  lw t0, 316(sp)
  sw t0, 336(sp)
  lw t0, 336(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 340(sp)
  lw t0, 340(sp)
  sw t0, 304(sp)
  j main_lor_end_21
main_lor_end_21:
  lw t0, 304(sp)
  sw t0, 344(sp)
  call f
  sw a0, 352(sp)
  lw t0, 352(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 356(sp)
  li t0, 1
  sw t0, 348(sp)
  lw t0, 356(sp)
  bnez t0, main_lor_end_25
  j main_lor_next_24
main_lor_next_24:
  call f
  sw a0, 364(sp)
  lw t0, 364(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 368(sp)
  sw x0, 360(sp)
  lw t0, 368(sp)
  bnez t0, main_land_right_26
  j main_land_end_27
main_land_right_26:
  call t
  sw a0, 372(sp)
  lw t0, 372(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 376(sp)
  lw t0, 376(sp)
  sw t0, 360(sp)
  j main_land_end_27
main_land_end_27:
  lw t0, 360(sp)
  sw t0, 380(sp)
  lw t0, 380(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 384(sp)
  lw t0, 384(sp)
  sw t0, 348(sp)
  j main_lor_end_25
main_lor_end_25:
  lw t0, 348(sp)
  sw t0, 388(sp)
  call t
  sw a0, 400(sp)
  lw t0, 400(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 404(sp)
  sw x0, 396(sp)
  lw t0, 404(sp)
  bnez t0, main_land_right_28
  j main_land_end_29
main_land_right_28:
  call t
  sw a0, 408(sp)
  lw t0, 408(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 412(sp)
  lw t0, 412(sp)
  sw t0, 396(sp)
  j main_land_end_29
main_land_end_29:
  lw t0, 396(sp)
  sw t0, 416(sp)
  lw t0, 416(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 420(sp)
  li t0, 1
  sw t0, 392(sp)
  lw t0, 420(sp)
  bnez t0, main_lor_end_31
  j main_lor_next_30
main_lor_next_30:
  call t
  sw a0, 424(sp)
  lw t0, 424(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 428(sp)
  lw t0, 428(sp)
  sw t0, 392(sp)
  j main_lor_end_31
main_lor_end_31:
  lw t0, 392(sp)
  sw t0, 432(sp)
  call f
  sw a0, 444(sp)
  lw t0, 444(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 448(sp)
  sw x0, 440(sp)
  lw t0, 448(sp)
  bnez t0, main_land_right_32
  j main_land_end_33
main_land_right_32:
  call t
  sw a0, 452(sp)
  lw t0, 452(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 456(sp)
  lw t0, 456(sp)
  sw t0, 440(sp)
  j main_land_end_33
main_land_end_33:
  lw t0, 440(sp)
  sw t0, 460(sp)
  lw t0, 460(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 464(sp)
  li t0, 1
  sw t0, 436(sp)
  lw t0, 464(sp)
  bnez t0, main_lor_end_35
  j main_lor_next_34
main_lor_next_34:
  call t
  sw a0, 468(sp)
  lw t0, 468(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 472(sp)
  lw t0, 472(sp)
  sw t0, 436(sp)
  j main_lor_end_35
main_lor_end_35:
  lw t0, 436(sp)
  sw t0, 476(sp)
  call f
  sw a0, 488(sp)
  lw t0, 488(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 492(sp)
  sw x0, 484(sp)
  lw t0, 492(sp)
  bnez t0, main_land_right_36
  j main_land_end_37
main_land_right_36:
  call f
  sw a0, 496(sp)
  lw t0, 496(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 500(sp)
  lw t0, 500(sp)
  sw t0, 484(sp)
  j main_land_end_37
main_land_end_37:
  lw t0, 484(sp)
  sw t0, 504(sp)
  lw t0, 504(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 508(sp)
  li t0, 1
  sw t0, 480(sp)
  lw t0, 508(sp)
  bnez t0, main_lor_end_39
  j main_lor_next_38
main_lor_next_38:
  call f
  sw a0, 512(sp)
  lw t0, 512(sp)
  xor t0, t0, x0
  snez t0, t0
  sw t0, 516(sp)
  lw t0, 516(sp)
  sw t0, 480(sp)
  j main_lor_end_39
main_lor_end_39:
  lw t0, 480(sp)
  sw t0, 520(sp)
  la t0, x
  lw t0, 0(t0)
  sw t0, 524(sp)
  lw a0, 524(sp)
  call putint
  li a0, 32
  call putch
  la t0, y
  lw t0, 0(t0)
  sw t0, 528(sp)
  lw a0, 528(sp)
  call putint
  li a0, 10
  call putch
  lw t0, 0(sp)
  sw t0, 532(sp)
  lw a0, 532(sp)
  lw ra, 540(sp)
  addi sp, sp, 544
  ret

