     10 REM La jaca Paca va para Alava y tu madre no lo dice pero me mira mal y ve el capitan pirata canatando alegre en la popa Asia a un lado al otro Europa.
     15 def int a-z
     20 GOSUB 9000
     30 PRINT USR (addr, 1, 10, 100, 1000)
     40 END
    980 REM
   1000 r= PEEK (a) + 256 * (PEEK (a+1) + 256 * (PEEK (a+2) + 256 * PEEK (a+3) ) )
   1010 RETURN
   9000 REM Inicio
   9001 RESTORE 10000 ' x86
   9002 rem RESTORE 20000 ' arm
   9010 p= PROGRAMPTR
   9020 a= p : GOSUB 1000 : n= r
   9030 IF n <> 10 THEN END
   9040 a= p + 4 : GOSUB 1000 : l= r
   9050 IF l <= 0 THEN END
   9053 REM A byte 0 terminates the listing of the line.
   9054 REM You can see an empty REM in line 10 after execution.
   9055 POKE p + 10, 0
   9060 addr= p + 11
   9070 p= addr
   9080 READ code$
   9090 rem PRINT code$; "-";
   9100 IF code$ = "xx" THEN RETURN
   9110 code= VAL ("&" + code$)
   9115 rem print hex$ (code, 2); "-";
   9120 POKE p, code
   9130 p= p + 1
   9140 GOTO 9080
  10000 REM
  10010 REM Machine code
  10020 REM I compiled this with gcc -S, then assembled with as -a
  10021 REM to obtain the codes (masters of assembler, excuse me).
  10022 REM int test (int nparams, int * param)
  10023 REM {
  10024 REM 	int r= 0;
  10025 REM 	for (int i= 0; i < nparams; ++i)
  10026 REM 		r+= param [i];
  10027 REM	return r;
  10028 REM }
  10030 DATA 55: REM pushl %ebp
  10040 DATA 89, e5: REM movl %esp, %ebp
  10050 DATA 83, ec, 18: REM subbl $24,%esp
  10060 DATA c7, 45, fc, 0, 0, 0, 0: REM movl $0,-4(%ebp)
  10070 DATA c7, 45, f8, 0, 0, 0, 0: REM movl $0,-8(%ebp)
  10075 REM .L3
  10080 DATA 8b, 45, f8: REM movl -8(%ebp),%eax
  10090 DATA 3b, 45, 08: REM cmpl 8(%ebp),%eax
  10100 DATA 7c, 04: REM jl .L6
  10110 DATA eb, 1a: REM jmp .L4
  10120 DATA 89, f6: REM .p2align 4,,7
  10125 REM .L6
  10130 DATA 8b, 45, f8: REM movl -8(%ebp),%eax
  10140 DATA 8d, 14, 85, 0, 0, 0, 0: REM leal 0(,%eax,4),%edx
  10150 DATA 8b, 45, 0c: REM movl 12(%ebp),%eax
  10160 DATA 8b, 14, 10: REM movl (%eax,%edx),%edx
  10170 DATA 01, 55, fc: REM addl %edx,-4(%ebp)
  10175 REM .L5
  10180 DATA ff, 45, f8: REM incl -8(%ebp)
  10190 DATA eb, dc: REM jmp .L3
  10195 REM .L4
  10200 DATA 8b, 55, fc: REM movl -4(%ebp),%edx
  10210 DATA 89, d0: REM movl %edx,%eax
  10220 DATA eb, 01: REM jmp .L2
  10230 DATA 90: REM .p2align 4,,7
  10235 REM .L2
  10240 DATA 89, ec: REM movl %ebp, %esp
  10250 DATA 5d: REM popl %ebp
  10260 DATA c3: REM ret
  11000 DATA xx: REM No more data
  20000 '
  20010 ' arm
  20020 '
  20030 DATA 0D, C0, A0, E1 ' mov	ip, sp
  20040 DATA 00, D8, 2D, E9 ' stmfd	sp!, {fp, ip, lr, pc}
  20050 DATA 04, B0, 4C, E2 ' sub	fp, ip, #4
  20060 DATA 10, D0, 4D, E2 ' sub	sp, sp, #16
  20070 DATA 10, 00, 0B, E5 ' str	r0, [fp, #-16]
  20080 DATA 14, 10, 0B, E5 ' str	r1, [fp, #-20]
  20090 DATA 00, 30, A0, E3 ' mov	r3, #0
  20100 DATA 18, 30, 0B, E5 ' str	r3, [fp, #-24]
  20110 DATA 00, 30, A0, E3 ' mov	r3, #0
  20120 DATA 1C, 30, 0B, E5 ' str	r3, [fp, #-28]
  20130 ' .L3:
  20130 DATA 1C, 30, 1B, E5 ' ldr	r3, [fp, #-28]
  20140 DATA 10, 20, 1B, E5 ' ldr	r2, [fp, #-16]
  20150 DATA 02, 00, 53, E1 ' cmp	r3, r2
  20160 DATA 0D, 00, 00, BA ' blt	.L6
  20170 DATA 1A, 00, 00, EA ' b	.L4
  20180 ' .L6:
  20180 DATA 1C, 30, 1B, E5 ' ldr	r3, [fp, #-28]
  20190 DATA 03, 20, A0, E1 ' mov	r2, r3
  20200 DATA 02, 31, A0, E1 ' mov	r3, r2, asl #2
  20210 DATA 14, 20, 1B, E5 ' ldr	r2, [fp, #-20]
  20220 DATA 02, 30, 83, E0 ' add	r3, r3, r2
  20230 DATA 18, 20, 1B, E5 ' ldr	r2, [fp, #-24]
  20240 DATA 00, 30, 93, E5 ' ldr	r3, [r3, #0]
  20250 DATA 03, 20, 82, E0 ' add	r2, r2, r3
  20260 DATA 18, 20, 0B, E5 ' str	r2, [fp, #-24]
  20270 ' .L5:
  20270 DATA 1C, 30, 1B, E5 ' ldr	r3, [fp, #-28]
  20280 DATA 01, 20, 83, E2 ' add	r2, r3, #1
  20290 DATA 1C, 20, 0B, E5 ' str	r2, [fp, #-28]
  20300 DATA 08, 00, 00, EA ' b	.L3
  20310 ' .L4:
  20310 DATA 18, 30, 1B, E5 ' ldr	r3, [fp, #-24]
  20320 DATA 03, 00, A0, E1 ' mov	r0, r3
  20330 DATA 1D, 00, 00, EA ' b	.L2
  20340 ' .L2:
  20340 DATA 00, A8, 1B, E9 ' ldmea	fp, {fp, sp, pc}
  20350 DATA xx
