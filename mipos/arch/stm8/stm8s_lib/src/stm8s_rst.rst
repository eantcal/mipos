                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ANSI-C Compiler
                                      3 ; Version 3.6.0 #9615 (Linux)
                                      4 ;--------------------------------------------------------
                                      5 	.module stm8s_rst
                                      6 	.optsdcc -mstm8
                                      7 	
                                      8 ;--------------------------------------------------------
                                      9 ; Public variables in this module
                                     10 ;--------------------------------------------------------
                                     11 	.globl _assert_failed
                                     12 	.globl _RST_GetFlagStatus
                                     13 	.globl _RST_ClearFlag
                                     14 ;--------------------------------------------------------
                                     15 ; ram data
                                     16 ;--------------------------------------------------------
                                     17 	.area DATA
                                     18 ;--------------------------------------------------------
                                     19 ; ram data
                                     20 ;--------------------------------------------------------
                                     21 	.area INITIALIZED
                                     22 ;--------------------------------------------------------
                                     23 ; absolute external ram data
                                     24 ;--------------------------------------------------------
                                     25 	.area DABS (ABS)
                                     26 ;--------------------------------------------------------
                                     27 ; global & static initialisations
                                     28 ;--------------------------------------------------------
                                     29 	.area HOME
                                     30 	.area GSINIT
                                     31 	.area GSFINAL
                                     32 	.area GSINIT
                                     33 ;--------------------------------------------------------
                                     34 ; Home
                                     35 ;--------------------------------------------------------
                                     36 	.area HOME
                                     37 	.area HOME
                                     38 ;--------------------------------------------------------
                                     39 ; code
                                     40 ;--------------------------------------------------------
                                     41 	.area CODE
                                     42 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_rst.c: 54: FlagStatus RST_GetFlagStatus(RST_Flag_TypeDef RST_Flag)
                                     43 ;	-----------------------------------------
                                     44 ;	 function RST_GetFlagStatus
                                     45 ;	-----------------------------------------
      0096E6                         46 _RST_GetFlagStatus:
                                     47 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_rst.c: 57: assert_param(IS_RST_FLAG_OK(RST_Flag));
      0096E6 7B 03            [ 1]   48 	ld	a, (0x03, sp)
      0096E8 A1 10            [ 1]   49 	cp	a, #0x10
      0096EA 27 29            [ 1]   50 	jreq	00104$
      0096EC 7B 03            [ 1]   51 	ld	a, (0x03, sp)
      0096EE A1 08            [ 1]   52 	cp	a, #0x08
      0096F0 27 23            [ 1]   53 	jreq	00104$
      0096F2 7B 03            [ 1]   54 	ld	a, (0x03, sp)
      0096F4 A1 04            [ 1]   55 	cp	a, #0x04
      0096F6 27 1D            [ 1]   56 	jreq	00104$
      0096F8 7B 03            [ 1]   57 	ld	a, (0x03, sp)
      0096FA A1 02            [ 1]   58 	cp	a, #0x02
      0096FC 27 17            [ 1]   59 	jreq	00104$
      0096FE 7B 03            [ 1]   60 	ld	a, (0x03, sp)
      009700 A1 01            [ 1]   61 	cp	a, #0x01
      009702 27 11            [ 1]   62 	jreq	00104$
      009704 AE 97 59         [ 2]   63 	ldw	x, #___str_0+0
      009707 4B 39            [ 1]   64 	push	#0x39
      009709 4B 00            [ 1]   65 	push	#0x00
      00970B 4B 00            [ 1]   66 	push	#0x00
      00970D 4B 00            [ 1]   67 	push	#0x00
      00970F 89               [ 2]   68 	pushw	x
      009710 CD 80 A8         [ 4]   69 	call	_assert_failed
      009713 5B 06            [ 2]   70 	addw	sp, #6
      009715                         71 00104$:
                                     72 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_rst.c: 60: return((FlagStatus)(((uint8_t)(RST->SR & RST_Flag) == (uint8_t)0x00) ? RESET : SET));
      009715 AE 50 B3         [ 2]   73 	ldw	x, #0x50b3
      009718 F6               [ 1]   74 	ld	a, (x)
      009719 14 03            [ 1]   75 	and	a, (0x03, sp)
      00971B 4D               [ 1]   76 	tnz	a
      00971C 26 02            [ 1]   77 	jrne	00117$
      00971E 4F               [ 1]   78 	clr	a
      00971F 81               [ 4]   79 	ret
      009720                         80 00117$:
      009720 A6 01            [ 1]   81 	ld	a, #0x01
      009722 81               [ 4]   82 	ret
                                     83 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_rst.c: 69: void RST_ClearFlag(RST_Flag_TypeDef RST_Flag)
                                     84 ;	-----------------------------------------
                                     85 ;	 function RST_ClearFlag
                                     86 ;	-----------------------------------------
      009723                         87 _RST_ClearFlag:
                                     88 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_rst.c: 72: assert_param(IS_RST_FLAG_OK(RST_Flag));
      009723 7B 03            [ 1]   89 	ld	a, (0x03, sp)
      009725 A1 10            [ 1]   90 	cp	a, #0x10
      009727 27 29            [ 1]   91 	jreq	00104$
      009729 7B 03            [ 1]   92 	ld	a, (0x03, sp)
      00972B A1 08            [ 1]   93 	cp	a, #0x08
      00972D 27 23            [ 1]   94 	jreq	00104$
      00972F 7B 03            [ 1]   95 	ld	a, (0x03, sp)
      009731 A1 04            [ 1]   96 	cp	a, #0x04
      009733 27 1D            [ 1]   97 	jreq	00104$
      009735 7B 03            [ 1]   98 	ld	a, (0x03, sp)
      009737 A1 02            [ 1]   99 	cp	a, #0x02
      009739 27 17            [ 1]  100 	jreq	00104$
      00973B 7B 03            [ 1]  101 	ld	a, (0x03, sp)
      00973D A1 01            [ 1]  102 	cp	a, #0x01
      00973F 27 11            [ 1]  103 	jreq	00104$
      009741 AE 97 59         [ 2]  104 	ldw	x, #___str_0+0
      009744 4B 48            [ 1]  105 	push	#0x48
      009746 4B 00            [ 1]  106 	push	#0x00
      009748 4B 00            [ 1]  107 	push	#0x00
      00974A 4B 00            [ 1]  108 	push	#0x00
      00974C 89               [ 2]  109 	pushw	x
      00974D CD 80 A8         [ 4]  110 	call	_assert_failed
      009750 5B 06            [ 2]  111 	addw	sp, #6
      009752                        112 00104$:
                                    113 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_rst.c: 74: RST->SR = (uint8_t)RST_Flag;
      009752 AE 50 B3         [ 2]  114 	ldw	x, #0x50b3
      009755 7B 03            [ 1]  115 	ld	a, (0x03, sp)
      009757 F7               [ 1]  116 	ld	(x), a
      009758 81               [ 4]  117 	ret
                                    118 	.area CODE
      009759                        119 ___str_0:
      009759 2E 2E 2F 2E 2E 2F 6D   120 	.ascii "../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_rst.c"
             69 70 6F 73 2F 61 72
             63 68 2F 73 74 6D 38
             2F 73 74 6D 38 73 2D
             73 64 63 63 2F 73 72
             63 2F 73 74 6D 38 73
             5F 72 73 74 2E 63
      009789 00                     121 	.db 0x00
                                    122 	.area INITIALIZER
                                    123 	.area CABS (ABS)
