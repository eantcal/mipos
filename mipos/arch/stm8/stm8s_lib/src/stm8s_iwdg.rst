                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ANSI-C Compiler
                                      3 ; Version 3.6.0 #9615 (Linux)
                                      4 ;--------------------------------------------------------
                                      5 	.module stm8s_iwdg
                                      6 	.optsdcc -mstm8
                                      7 	
                                      8 ;--------------------------------------------------------
                                      9 ; Public variables in this module
                                     10 ;--------------------------------------------------------
                                     11 	.globl _assert_failed
                                     12 	.globl _IWDG_WriteAccessCmd
                                     13 	.globl _IWDG_SetPrescaler
                                     14 	.globl _IWDG_SetReload
                                     15 	.globl _IWDG_ReloadCounter
                                     16 	.globl _IWDG_Enable
                                     17 ;--------------------------------------------------------
                                     18 ; ram data
                                     19 ;--------------------------------------------------------
                                     20 	.area DATA
                                     21 ;--------------------------------------------------------
                                     22 ; ram data
                                     23 ;--------------------------------------------------------
                                     24 	.area INITIALIZED
                                     25 ;--------------------------------------------------------
                                     26 ; absolute external ram data
                                     27 ;--------------------------------------------------------
                                     28 	.area DABS (ABS)
                                     29 ;--------------------------------------------------------
                                     30 ; global & static initialisations
                                     31 ;--------------------------------------------------------
                                     32 	.area HOME
                                     33 	.area GSINIT
                                     34 	.area GSFINAL
                                     35 	.area GSINIT
                                     36 ;--------------------------------------------------------
                                     37 ; Home
                                     38 ;--------------------------------------------------------
                                     39 	.area HOME
                                     40 	.area HOME
                                     41 ;--------------------------------------------------------
                                     42 ; code
                                     43 ;--------------------------------------------------------
                                     44 	.area CODE
                                     45 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 48: void IWDG_WriteAccessCmd(IWDG_WriteAccess_TypeDef IWDG_WriteAccess)
                                     46 ;	-----------------------------------------
                                     47 ;	 function IWDG_WriteAccessCmd
                                     48 ;	-----------------------------------------
      00978A                         49 _IWDG_WriteAccessCmd:
                                     50 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 51: assert_param(IS_IWDG_WRITEACCESS_MODE_OK(IWDG_WriteAccess));
      00978A 7B 03            [ 1]   51 	ld	a, (0x03, sp)
      00978C A1 55            [ 1]   52 	cp	a, #0x55
      00978E 27 15            [ 1]   53 	jreq	00104$
      009790 0D 03            [ 1]   54 	tnz	(0x03, sp)
      009792 27 11            [ 1]   55 	jreq	00104$
      009794 AE 97 FD         [ 2]   56 	ldw	x, #___str_0+0
      009797 4B 33            [ 1]   57 	push	#0x33
      009799 4B 00            [ 1]   58 	push	#0x00
      00979B 4B 00            [ 1]   59 	push	#0x00
      00979D 4B 00            [ 1]   60 	push	#0x00
      00979F 89               [ 2]   61 	pushw	x
      0097A0 CD 80 A8         [ 4]   62 	call	_assert_failed
      0097A3 5B 06            [ 2]   63 	addw	sp, #6
      0097A5                         64 00104$:
                                     65 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 53: IWDG->KR = (uint8_t)IWDG_WriteAccess; /* Write Access */
      0097A5 AE 50 E0         [ 2]   66 	ldw	x, #0x50e0
      0097A8 7B 03            [ 1]   67 	ld	a, (0x03, sp)
      0097AA F7               [ 1]   68 	ld	(x), a
      0097AB 81               [ 4]   69 	ret
                                     70 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 63: void IWDG_SetPrescaler(IWDG_Prescaler_TypeDef IWDG_Prescaler)
                                     71 ;	-----------------------------------------
                                     72 ;	 function IWDG_SetPrescaler
                                     73 ;	-----------------------------------------
      0097AC                         74 _IWDG_SetPrescaler:
                                     75 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 66: assert_param(IS_IWDG_PRESCALER_OK(IWDG_Prescaler));
      0097AC 0D 03            [ 1]   76 	tnz	(0x03, sp)
      0097AE 27 35            [ 1]   77 	jreq	00104$
      0097B0 7B 03            [ 1]   78 	ld	a, (0x03, sp)
      0097B2 A1 01            [ 1]   79 	cp	a, #0x01
      0097B4 27 2F            [ 1]   80 	jreq	00104$
      0097B6 7B 03            [ 1]   81 	ld	a, (0x03, sp)
      0097B8 A1 02            [ 1]   82 	cp	a, #0x02
      0097BA 27 29            [ 1]   83 	jreq	00104$
      0097BC 7B 03            [ 1]   84 	ld	a, (0x03, sp)
      0097BE A1 03            [ 1]   85 	cp	a, #0x03
      0097C0 27 23            [ 1]   86 	jreq	00104$
      0097C2 7B 03            [ 1]   87 	ld	a, (0x03, sp)
      0097C4 A1 04            [ 1]   88 	cp	a, #0x04
      0097C6 27 1D            [ 1]   89 	jreq	00104$
      0097C8 7B 03            [ 1]   90 	ld	a, (0x03, sp)
      0097CA A1 05            [ 1]   91 	cp	a, #0x05
      0097CC 27 17            [ 1]   92 	jreq	00104$
      0097CE 7B 03            [ 1]   93 	ld	a, (0x03, sp)
      0097D0 A1 06            [ 1]   94 	cp	a, #0x06
      0097D2 27 11            [ 1]   95 	jreq	00104$
      0097D4 AE 97 FD         [ 2]   96 	ldw	x, #___str_0+0
      0097D7 4B 42            [ 1]   97 	push	#0x42
      0097D9 4B 00            [ 1]   98 	push	#0x00
      0097DB 4B 00            [ 1]   99 	push	#0x00
      0097DD 4B 00            [ 1]  100 	push	#0x00
      0097DF 89               [ 2]  101 	pushw	x
      0097E0 CD 80 A8         [ 4]  102 	call	_assert_failed
      0097E3 5B 06            [ 2]  103 	addw	sp, #6
      0097E5                        104 00104$:
                                    105 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 68: IWDG->PR = (uint8_t)IWDG_Prescaler;
      0097E5 AE 50 E1         [ 2]  106 	ldw	x, #0x50e1
      0097E8 7B 03            [ 1]  107 	ld	a, (0x03, sp)
      0097EA F7               [ 1]  108 	ld	(x), a
      0097EB 81               [ 4]  109 	ret
                                    110 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 78: void IWDG_SetReload(uint8_t IWDG_Reload)
                                    111 ;	-----------------------------------------
                                    112 ;	 function IWDG_SetReload
                                    113 ;	-----------------------------------------
      0097EC                        114 _IWDG_SetReload:
                                    115 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 80: IWDG->RLR = IWDG_Reload;
      0097EC AE 50 E2         [ 2]  116 	ldw	x, #0x50e2
      0097EF 7B 03            [ 1]  117 	ld	a, (0x03, sp)
      0097F1 F7               [ 1]  118 	ld	(x), a
      0097F2 81               [ 4]  119 	ret
                                    120 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 89: void IWDG_ReloadCounter(void)
                                    121 ;	-----------------------------------------
                                    122 ;	 function IWDG_ReloadCounter
                                    123 ;	-----------------------------------------
      0097F3                        124 _IWDG_ReloadCounter:
                                    125 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 91: IWDG->KR = IWDG_KEY_REFRESH;
      0097F3 35 AA 50 E0      [ 1]  126 	mov	0x50e0+0, #0xaa
      0097F7 81               [ 4]  127 	ret
                                    128 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 99: void IWDG_Enable(void)
                                    129 ;	-----------------------------------------
                                    130 ;	 function IWDG_Enable
                                    131 ;	-----------------------------------------
      0097F8                        132 _IWDG_Enable:
                                    133 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c: 101: IWDG->KR = IWDG_KEY_ENABLE;
      0097F8 35 CC 50 E0      [ 1]  134 	mov	0x50e0+0, #0xcc
      0097FC 81               [ 4]  135 	ret
                                    136 	.area CODE
      0097FD                        137 ___str_0:
      0097FD 2E 2E 2F 2E 2E 2F 6D   138 	.ascii "../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_iwdg.c"
             69 70 6F 73 2F 61 72
             63 68 2F 73 74 6D 38
             2F 73 74 6D 38 73 2D
             73 64 63 63 2F 73 72
             63 2F 73 74 6D 38 73
             5F 69 77 64 67 2E 63
      00982E 00                     139 	.db 0x00
                                    140 	.area INITIALIZER
                                    141 	.area CABS (ABS)
