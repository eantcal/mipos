                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ANSI-C Compiler
                                      3 ; Version 3.6.0 #9615 (Linux)
                                      4 ;--------------------------------------------------------
                                      5 	.module stm8s_clk
                                      6 	.optsdcc -mstm8
                                      7 	
                                      8 ;--------------------------------------------------------
                                      9 ; Public variables in this module
                                     10 ;--------------------------------------------------------
                                     11 	.globl _CLKPrescTable
                                     12 	.globl _HSIDivFactor
                                     13 	.globl _assert_failed
                                     14 	.globl _CLK_DeInit
                                     15 	.globl _CLK_FastHaltWakeUpCmd
                                     16 	.globl _CLK_HSECmd
                                     17 	.globl _CLK_HSICmd
                                     18 	.globl _CLK_LSICmd
                                     19 	.globl _CLK_CCOCmd
                                     20 	.globl _CLK_ClockSwitchCmd
                                     21 	.globl _CLK_SlowActiveHaltWakeUpCmd
                                     22 	.globl _CLK_PeripheralClockConfig
                                     23 	.globl _CLK_ClockSwitchConfig
                                     24 	.globl _CLK_HSIPrescalerConfig
                                     25 	.globl _CLK_CCOConfig
                                     26 	.globl _CLK_ITConfig
                                     27 	.globl _CLK_SYSCLKConfig
                                     28 	.globl _CLK_SWIMConfig
                                     29 	.globl _CLK_ClockSecuritySystemEnable
                                     30 	.globl _CLK_GetSYSCLKSource
                                     31 	.globl _CLK_GetClockFreq
                                     32 	.globl _CLK_AdjustHSICalibrationValue
                                     33 	.globl _CLK_SYSCLKEmergencyClear
                                     34 	.globl _CLK_GetFlagStatus
                                     35 	.globl _CLK_GetITStatus
                                     36 	.globl _CLK_ClearITPendingBit
                                     37 ;--------------------------------------------------------
                                     38 ; ram data
                                     39 ;--------------------------------------------------------
                                     40 	.area DATA
                                     41 ;--------------------------------------------------------
                                     42 ; ram data
                                     43 ;--------------------------------------------------------
                                     44 	.area INITIALIZED
                                     45 ;--------------------------------------------------------
                                     46 ; absolute external ram data
                                     47 ;--------------------------------------------------------
                                     48 	.area DABS (ABS)
                                     49 ;--------------------------------------------------------
                                     50 ; global & static initialisations
                                     51 ;--------------------------------------------------------
                                     52 	.area HOME
                                     53 	.area GSINIT
                                     54 	.area GSFINAL
                                     55 	.area GSINIT
                                     56 ;--------------------------------------------------------
                                     57 ; Home
                                     58 ;--------------------------------------------------------
                                     59 	.area HOME
                                     60 	.area HOME
                                     61 ;--------------------------------------------------------
                                     62 ; code
                                     63 ;--------------------------------------------------------
                                     64 	.area CODE
                                     65 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 72: void CLK_DeInit(void)
                                     66 ;	-----------------------------------------
                                     67 ;	 function CLK_DeInit
                                     68 ;	-----------------------------------------
      008F3C                         69 _CLK_DeInit:
                                     70 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 74: CLK->ICKR = CLK_ICKR_RESET_VALUE;
      008F3C 35 01 50 C0      [ 1]   71 	mov	0x50c0+0, #0x01
                                     72 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 75: CLK->ECKR = CLK_ECKR_RESET_VALUE;
      008F40 35 00 50 C1      [ 1]   73 	mov	0x50c1+0, #0x00
                                     74 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 76: CLK->SWR  = CLK_SWR_RESET_VALUE;
      008F44 35 E1 50 C4      [ 1]   75 	mov	0x50c4+0, #0xe1
                                     76 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 77: CLK->SWCR = CLK_SWCR_RESET_VALUE;
      008F48 35 00 50 C5      [ 1]   77 	mov	0x50c5+0, #0x00
                                     78 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 78: CLK->CKDIVR = CLK_CKDIVR_RESET_VALUE;
      008F4C 35 18 50 C6      [ 1]   79 	mov	0x50c6+0, #0x18
                                     80 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 79: CLK->PCKENR1 = CLK_PCKENR1_RESET_VALUE;
      008F50 35 FF 50 C7      [ 1]   81 	mov	0x50c7+0, #0xff
                                     82 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 80: CLK->PCKENR2 = CLK_PCKENR2_RESET_VALUE;
      008F54 35 FF 50 CA      [ 1]   83 	mov	0x50ca+0, #0xff
                                     84 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 81: CLK->CSSR = CLK_CSSR_RESET_VALUE;
      008F58 35 00 50 C8      [ 1]   85 	mov	0x50c8+0, #0x00
                                     86 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 82: CLK->CCOR = CLK_CCOR_RESET_VALUE;
      008F5C 35 00 50 C9      [ 1]   87 	mov	0x50c9+0, #0x00
                                     88 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 83: while ((CLK->CCOR & CLK_CCOR_CCOEN)!= 0)
      008F60                         89 00101$:
      008F60 AE 50 C9         [ 2]   90 	ldw	x, #0x50c9
      008F63 F6               [ 1]   91 	ld	a, (x)
      008F64 44               [ 1]   92 	srl	a
      008F65 25 F9            [ 1]   93 	jrc	00101$
                                     94 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 85: CLK->CCOR = CLK_CCOR_RESET_VALUE;
      008F67 35 00 50 C9      [ 1]   95 	mov	0x50c9+0, #0x00
                                     96 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 86: CLK->HSITRIMR = CLK_HSITRIMR_RESET_VALUE;
      008F6B 35 00 50 CC      [ 1]   97 	mov	0x50cc+0, #0x00
                                     98 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 87: CLK->SWIMCCR = CLK_SWIMCCR_RESET_VALUE;
      008F6F 35 00 50 CD      [ 1]   99 	mov	0x50cd+0, #0x00
      008F73 81               [ 4]  100 	ret
                                    101 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 99: void CLK_FastHaltWakeUpCmd(FunctionalState NewState)
                                    102 ;	-----------------------------------------
                                    103 ;	 function CLK_FastHaltWakeUpCmd
                                    104 ;	-----------------------------------------
      008F74                        105 _CLK_FastHaltWakeUpCmd:
                                    106 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 102: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008F74 0D 03            [ 1]  107 	tnz	(0x03, sp)
      008F76 27 17            [ 1]  108 	jreq	00107$
      008F78 7B 03            [ 1]  109 	ld	a, (0x03, sp)
      008F7A A1 01            [ 1]  110 	cp	a, #0x01
      008F7C 27 11            [ 1]  111 	jreq	00107$
      008F7E AE 96 B5         [ 2]  112 	ldw	x, #___str_0+0
      008F81 4B 66            [ 1]  113 	push	#0x66
      008F83 4B 00            [ 1]  114 	push	#0x00
      008F85 4B 00            [ 1]  115 	push	#0x00
      008F87 4B 00            [ 1]  116 	push	#0x00
      008F89 89               [ 2]  117 	pushw	x
      008F8A CD 80 A8         [ 4]  118 	call	_assert_failed
      008F8D 5B 06            [ 2]  119 	addw	sp, #6
      008F8F                        120 00107$:
                                    121 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 104: if (NewState != DISABLE)
      008F8F 0D 03            [ 1]  122 	tnz	(0x03, sp)
      008F91 27 08            [ 1]  123 	jreq	00102$
                                    124 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 107: CLK->ICKR |= CLK_ICKR_FHWU;
      008F93 AE 50 C0         [ 2]  125 	ldw	x, #0x50c0
      008F96 F6               [ 1]  126 	ld	a, (x)
      008F97 AA 04            [ 1]  127 	or	a, #0x04
      008F99 F7               [ 1]  128 	ld	(x), a
      008F9A 81               [ 4]  129 	ret
      008F9B                        130 00102$:
                                    131 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 112: CLK->ICKR &= (uint8_t)(~CLK_ICKR_FHWU);
      008F9B AE 50 C0         [ 2]  132 	ldw	x, #0x50c0
      008F9E F6               [ 1]  133 	ld	a, (x)
      008F9F A4 FB            [ 1]  134 	and	a, #0xfb
      008FA1 F7               [ 1]  135 	ld	(x), a
      008FA2 81               [ 4]  136 	ret
                                    137 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 121: void CLK_HSECmd(FunctionalState NewState)
                                    138 ;	-----------------------------------------
                                    139 ;	 function CLK_HSECmd
                                    140 ;	-----------------------------------------
      008FA3                        141 _CLK_HSECmd:
                                    142 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 124: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008FA3 0D 03            [ 1]  143 	tnz	(0x03, sp)
      008FA5 27 17            [ 1]  144 	jreq	00107$
      008FA7 7B 03            [ 1]  145 	ld	a, (0x03, sp)
      008FA9 A1 01            [ 1]  146 	cp	a, #0x01
      008FAB 27 11            [ 1]  147 	jreq	00107$
      008FAD AE 96 B5         [ 2]  148 	ldw	x, #___str_0+0
      008FB0 4B 7C            [ 1]  149 	push	#0x7c
      008FB2 4B 00            [ 1]  150 	push	#0x00
      008FB4 4B 00            [ 1]  151 	push	#0x00
      008FB6 4B 00            [ 1]  152 	push	#0x00
      008FB8 89               [ 2]  153 	pushw	x
      008FB9 CD 80 A8         [ 4]  154 	call	_assert_failed
      008FBC 5B 06            [ 2]  155 	addw	sp, #6
      008FBE                        156 00107$:
                                    157 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 126: if (NewState != DISABLE)
      008FBE 0D 03            [ 1]  158 	tnz	(0x03, sp)
      008FC0 27 05            [ 1]  159 	jreq	00102$
                                    160 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 129: CLK->ECKR |= CLK_ECKR_HSEEN;
      008FC2 72 10 50 C1      [ 1]  161 	bset	0x50c1, #0
      008FC6 81               [ 4]  162 	ret
      008FC7                        163 00102$:
                                    164 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 134: CLK->ECKR &= (uint8_t)(~CLK_ECKR_HSEEN);
      008FC7 72 11 50 C1      [ 1]  165 	bres	0x50c1, #0
      008FCB 81               [ 4]  166 	ret
                                    167 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 143: void CLK_HSICmd(FunctionalState NewState)
                                    168 ;	-----------------------------------------
                                    169 ;	 function CLK_HSICmd
                                    170 ;	-----------------------------------------
      008FCC                        171 _CLK_HSICmd:
                                    172 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 146: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008FCC 0D 03            [ 1]  173 	tnz	(0x03, sp)
      008FCE 27 17            [ 1]  174 	jreq	00107$
      008FD0 7B 03            [ 1]  175 	ld	a, (0x03, sp)
      008FD2 A1 01            [ 1]  176 	cp	a, #0x01
      008FD4 27 11            [ 1]  177 	jreq	00107$
      008FD6 AE 96 B5         [ 2]  178 	ldw	x, #___str_0+0
      008FD9 4B 92            [ 1]  179 	push	#0x92
      008FDB 4B 00            [ 1]  180 	push	#0x00
      008FDD 4B 00            [ 1]  181 	push	#0x00
      008FDF 4B 00            [ 1]  182 	push	#0x00
      008FE1 89               [ 2]  183 	pushw	x
      008FE2 CD 80 A8         [ 4]  184 	call	_assert_failed
      008FE5 5B 06            [ 2]  185 	addw	sp, #6
      008FE7                        186 00107$:
                                    187 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 148: if (NewState != DISABLE)
      008FE7 0D 03            [ 1]  188 	tnz	(0x03, sp)
      008FE9 27 05            [ 1]  189 	jreq	00102$
                                    190 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 151: CLK->ICKR |= CLK_ICKR_HSIEN;
      008FEB 72 10 50 C0      [ 1]  191 	bset	0x50c0, #0
      008FEF 81               [ 4]  192 	ret
      008FF0                        193 00102$:
                                    194 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 156: CLK->ICKR &= (uint8_t)(~CLK_ICKR_HSIEN);
      008FF0 72 11 50 C0      [ 1]  195 	bres	0x50c0, #0
      008FF4 81               [ 4]  196 	ret
                                    197 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 166: void CLK_LSICmd(FunctionalState NewState)
                                    198 ;	-----------------------------------------
                                    199 ;	 function CLK_LSICmd
                                    200 ;	-----------------------------------------
      008FF5                        201 _CLK_LSICmd:
                                    202 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 169: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008FF5 0D 03            [ 1]  203 	tnz	(0x03, sp)
      008FF7 27 17            [ 1]  204 	jreq	00107$
      008FF9 7B 03            [ 1]  205 	ld	a, (0x03, sp)
      008FFB A1 01            [ 1]  206 	cp	a, #0x01
      008FFD 27 11            [ 1]  207 	jreq	00107$
      008FFF AE 96 B5         [ 2]  208 	ldw	x, #___str_0+0
      009002 4B A9            [ 1]  209 	push	#0xa9
      009004 4B 00            [ 1]  210 	push	#0x00
      009006 4B 00            [ 1]  211 	push	#0x00
      009008 4B 00            [ 1]  212 	push	#0x00
      00900A 89               [ 2]  213 	pushw	x
      00900B CD 80 A8         [ 4]  214 	call	_assert_failed
      00900E 5B 06            [ 2]  215 	addw	sp, #6
      009010                        216 00107$:
                                    217 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 171: if (NewState != DISABLE)
      009010 0D 03            [ 1]  218 	tnz	(0x03, sp)
      009012 27 08            [ 1]  219 	jreq	00102$
                                    220 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 174: CLK->ICKR |= CLK_ICKR_LSIEN;
      009014 AE 50 C0         [ 2]  221 	ldw	x, #0x50c0
      009017 F6               [ 1]  222 	ld	a, (x)
      009018 AA 08            [ 1]  223 	or	a, #0x08
      00901A F7               [ 1]  224 	ld	(x), a
      00901B 81               [ 4]  225 	ret
      00901C                        226 00102$:
                                    227 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 179: CLK->ICKR &= (uint8_t)(~CLK_ICKR_LSIEN);
      00901C AE 50 C0         [ 2]  228 	ldw	x, #0x50c0
      00901F F6               [ 1]  229 	ld	a, (x)
      009020 A4 F7            [ 1]  230 	and	a, #0xf7
      009022 F7               [ 1]  231 	ld	(x), a
      009023 81               [ 4]  232 	ret
                                    233 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 189: void CLK_CCOCmd(FunctionalState NewState)
                                    234 ;	-----------------------------------------
                                    235 ;	 function CLK_CCOCmd
                                    236 ;	-----------------------------------------
      009024                        237 _CLK_CCOCmd:
                                    238 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 192: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      009024 0D 03            [ 1]  239 	tnz	(0x03, sp)
      009026 27 17            [ 1]  240 	jreq	00107$
      009028 7B 03            [ 1]  241 	ld	a, (0x03, sp)
      00902A A1 01            [ 1]  242 	cp	a, #0x01
      00902C 27 11            [ 1]  243 	jreq	00107$
      00902E AE 96 B5         [ 2]  244 	ldw	x, #___str_0+0
      009031 4B C0            [ 1]  245 	push	#0xc0
      009033 4B 00            [ 1]  246 	push	#0x00
      009035 4B 00            [ 1]  247 	push	#0x00
      009037 4B 00            [ 1]  248 	push	#0x00
      009039 89               [ 2]  249 	pushw	x
      00903A CD 80 A8         [ 4]  250 	call	_assert_failed
      00903D 5B 06            [ 2]  251 	addw	sp, #6
      00903F                        252 00107$:
                                    253 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 194: if (NewState != DISABLE)
      00903F 0D 03            [ 1]  254 	tnz	(0x03, sp)
      009041 27 05            [ 1]  255 	jreq	00102$
                                    256 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 197: CLK->CCOR |= CLK_CCOR_CCOEN;
      009043 72 10 50 C9      [ 1]  257 	bset	0x50c9, #0
      009047 81               [ 4]  258 	ret
      009048                        259 00102$:
                                    260 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 202: CLK->CCOR &= (uint8_t)(~CLK_CCOR_CCOEN);
      009048 72 11 50 C9      [ 1]  261 	bres	0x50c9, #0
      00904C 81               [ 4]  262 	ret
                                    263 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 213: void CLK_ClockSwitchCmd(FunctionalState NewState)
                                    264 ;	-----------------------------------------
                                    265 ;	 function CLK_ClockSwitchCmd
                                    266 ;	-----------------------------------------
      00904D                        267 _CLK_ClockSwitchCmd:
                                    268 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 216: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      00904D 0D 03            [ 1]  269 	tnz	(0x03, sp)
      00904F 27 17            [ 1]  270 	jreq	00107$
      009051 7B 03            [ 1]  271 	ld	a, (0x03, sp)
      009053 A1 01            [ 1]  272 	cp	a, #0x01
      009055 27 11            [ 1]  273 	jreq	00107$
      009057 AE 96 B5         [ 2]  274 	ldw	x, #___str_0+0
      00905A 4B D8            [ 1]  275 	push	#0xd8
      00905C 4B 00            [ 1]  276 	push	#0x00
      00905E 4B 00            [ 1]  277 	push	#0x00
      009060 4B 00            [ 1]  278 	push	#0x00
      009062 89               [ 2]  279 	pushw	x
      009063 CD 80 A8         [ 4]  280 	call	_assert_failed
      009066 5B 06            [ 2]  281 	addw	sp, #6
      009068                        282 00107$:
                                    283 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 218: if (NewState != DISABLE )
      009068 0D 03            [ 1]  284 	tnz	(0x03, sp)
      00906A 27 08            [ 1]  285 	jreq	00102$
                                    286 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 221: CLK->SWCR |= CLK_SWCR_SWEN;
      00906C AE 50 C5         [ 2]  287 	ldw	x, #0x50c5
      00906F F6               [ 1]  288 	ld	a, (x)
      009070 AA 02            [ 1]  289 	or	a, #0x02
      009072 F7               [ 1]  290 	ld	(x), a
      009073 81               [ 4]  291 	ret
      009074                        292 00102$:
                                    293 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 226: CLK->SWCR &= (uint8_t)(~CLK_SWCR_SWEN);
      009074 AE 50 C5         [ 2]  294 	ldw	x, #0x50c5
      009077 F6               [ 1]  295 	ld	a, (x)
      009078 A4 FD            [ 1]  296 	and	a, #0xfd
      00907A F7               [ 1]  297 	ld	(x), a
      00907B 81               [ 4]  298 	ret
                                    299 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 238: void CLK_SlowActiveHaltWakeUpCmd(FunctionalState NewState)
                                    300 ;	-----------------------------------------
                                    301 ;	 function CLK_SlowActiveHaltWakeUpCmd
                                    302 ;	-----------------------------------------
      00907C                        303 _CLK_SlowActiveHaltWakeUpCmd:
                                    304 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 241: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      00907C 0D 03            [ 1]  305 	tnz	(0x03, sp)
      00907E 27 17            [ 1]  306 	jreq	00107$
      009080 7B 03            [ 1]  307 	ld	a, (0x03, sp)
      009082 A1 01            [ 1]  308 	cp	a, #0x01
      009084 27 11            [ 1]  309 	jreq	00107$
      009086 AE 96 B5         [ 2]  310 	ldw	x, #___str_0+0
      009089 4B F1            [ 1]  311 	push	#0xf1
      00908B 4B 00            [ 1]  312 	push	#0x00
      00908D 4B 00            [ 1]  313 	push	#0x00
      00908F 4B 00            [ 1]  314 	push	#0x00
      009091 89               [ 2]  315 	pushw	x
      009092 CD 80 A8         [ 4]  316 	call	_assert_failed
      009095 5B 06            [ 2]  317 	addw	sp, #6
      009097                        318 00107$:
                                    319 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 243: if (NewState != DISABLE)
      009097 0D 03            [ 1]  320 	tnz	(0x03, sp)
      009099 27 08            [ 1]  321 	jreq	00102$
                                    322 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 246: CLK->ICKR |= CLK_ICKR_SWUAH;
      00909B AE 50 C0         [ 2]  323 	ldw	x, #0x50c0
      00909E F6               [ 1]  324 	ld	a, (x)
      00909F AA 20            [ 1]  325 	or	a, #0x20
      0090A1 F7               [ 1]  326 	ld	(x), a
      0090A2 81               [ 4]  327 	ret
      0090A3                        328 00102$:
                                    329 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 251: CLK->ICKR &= (uint8_t)(~CLK_ICKR_SWUAH);
      0090A3 AE 50 C0         [ 2]  330 	ldw	x, #0x50c0
      0090A6 F6               [ 1]  331 	ld	a, (x)
      0090A7 A4 DF            [ 1]  332 	and	a, #0xdf
      0090A9 F7               [ 1]  333 	ld	(x), a
      0090AA 81               [ 4]  334 	ret
                                    335 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 263: void CLK_PeripheralClockConfig(CLK_Peripheral_TypeDef CLK_Peripheral, FunctionalState NewState)
                                    336 ;	-----------------------------------------
                                    337 ;	 function CLK_PeripheralClockConfig
                                    338 ;	-----------------------------------------
      0090AB                        339 _CLK_PeripheralClockConfig:
      0090AB 52 02            [ 2]  340 	sub	sp, #2
                                    341 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 266: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      0090AD 0D 06            [ 1]  342 	tnz	(0x06, sp)
      0090AF 27 17            [ 1]  343 	jreq	00113$
      0090B1 7B 06            [ 1]  344 	ld	a, (0x06, sp)
      0090B3 A1 01            [ 1]  345 	cp	a, #0x01
      0090B5 27 11            [ 1]  346 	jreq	00113$
      0090B7 AE 96 B5         [ 2]  347 	ldw	x, #___str_0+0
      0090BA 4B 0A            [ 1]  348 	push	#0x0a
      0090BC 4B 01            [ 1]  349 	push	#0x01
      0090BE 4B 00            [ 1]  350 	push	#0x00
      0090C0 4B 00            [ 1]  351 	push	#0x00
      0090C2 89               [ 2]  352 	pushw	x
      0090C3 CD 80 A8         [ 4]  353 	call	_assert_failed
      0090C6 5B 06            [ 2]  354 	addw	sp, #6
      0090C8                        355 00113$:
                                    356 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 267: assert_param(IS_CLK_PERIPHERAL_OK(CLK_Peripheral));
      0090C8 0D 05            [ 1]  357 	tnz	(0x05, sp)
      0090CA 27 70            [ 1]  358 	jreq	00118$
      0090CC 7B 05            [ 1]  359 	ld	a, (0x05, sp)
      0090CE A1 01            [ 1]  360 	cp	a, #0x01
      0090D0 27 6A            [ 1]  361 	jreq	00118$
      0090D2 7B 05            [ 1]  362 	ld	a, (0x05, sp)
      0090D4 A1 03            [ 1]  363 	cp	a, #0x03
      0090D6 26 03            [ 1]  364 	jrne	00243$
      0090D8 A6 01            [ 1]  365 	ld	a, #0x01
      0090DA 21                     366 	.byte 0x21
      0090DB                        367 00243$:
      0090DB 4F               [ 1]  368 	clr	a
      0090DC                        369 00244$:
      0090DC 4D               [ 1]  370 	tnz	a
      0090DD 26 5D            [ 1]  371 	jrne	00118$
      0090DF 4D               [ 1]  372 	tnz	a
      0090E0 26 5A            [ 1]  373 	jrne	00118$
      0090E2 7B 05            [ 1]  374 	ld	a, (0x05, sp)
      0090E4 A1 02            [ 1]  375 	cp	a, #0x02
      0090E6 27 54            [ 1]  376 	jreq	00118$
      0090E8 7B 05            [ 1]  377 	ld	a, (0x05, sp)
      0090EA A1 04            [ 1]  378 	cp	a, #0x04
      0090EC 26 05            [ 1]  379 	jrne	00251$
      0090EE A6 01            [ 1]  380 	ld	a, #0x01
      0090F0 97               [ 1]  381 	ld	xl, a
      0090F1 20 02            [ 2]  382 	jra	00252$
      0090F3                        383 00251$:
      0090F3 4F               [ 1]  384 	clr	a
      0090F4 97               [ 1]  385 	ld	xl, a
      0090F5                        386 00252$:
      0090F5 9F               [ 1]  387 	ld	a, xl
      0090F6 4D               [ 1]  388 	tnz	a
      0090F7 26 43            [ 1]  389 	jrne	00118$
      0090F9 7B 05            [ 1]  390 	ld	a, (0x05, sp)
      0090FB A1 05            [ 1]  391 	cp	a, #0x05
      0090FD 26 03            [ 1]  392 	jrne	00255$
      0090FF A6 01            [ 1]  393 	ld	a, #0x01
      009101 21                     394 	.byte 0x21
      009102                        395 00255$:
      009102 4F               [ 1]  396 	clr	a
      009103                        397 00256$:
      009103 4D               [ 1]  398 	tnz	a
      009104 26 36            [ 1]  399 	jrne	00118$
      009106 4D               [ 1]  400 	tnz	a
      009107 26 33            [ 1]  401 	jrne	00118$
      009109 9F               [ 1]  402 	ld	a, xl
      00910A 4D               [ 1]  403 	tnz	a
      00910B 26 2F            [ 1]  404 	jrne	00118$
      00910D 7B 05            [ 1]  405 	ld	a, (0x05, sp)
      00910F A1 06            [ 1]  406 	cp	a, #0x06
      009111 27 29            [ 1]  407 	jreq	00118$
      009113 7B 05            [ 1]  408 	ld	a, (0x05, sp)
      009115 A1 07            [ 1]  409 	cp	a, #0x07
      009117 27 23            [ 1]  410 	jreq	00118$
      009119 7B 05            [ 1]  411 	ld	a, (0x05, sp)
      00911B A1 17            [ 1]  412 	cp	a, #0x17
      00911D 27 1D            [ 1]  413 	jreq	00118$
      00911F 7B 05            [ 1]  414 	ld	a, (0x05, sp)
      009121 A1 13            [ 1]  415 	cp	a, #0x13
      009123 27 17            [ 1]  416 	jreq	00118$
      009125 7B 05            [ 1]  417 	ld	a, (0x05, sp)
      009127 A1 12            [ 1]  418 	cp	a, #0x12
      009129 27 11            [ 1]  419 	jreq	00118$
      00912B AE 96 B5         [ 2]  420 	ldw	x, #___str_0+0
      00912E 4B 0B            [ 1]  421 	push	#0x0b
      009130 4B 01            [ 1]  422 	push	#0x01
      009132 4B 00            [ 1]  423 	push	#0x00
      009134 4B 00            [ 1]  424 	push	#0x00
      009136 89               [ 2]  425 	pushw	x
      009137 CD 80 A8         [ 4]  426 	call	_assert_failed
      00913A 5B 06            [ 2]  427 	addw	sp, #6
      00913C                        428 00118$:
                                    429 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 274: CLK->PCKENR1 |= (uint8_t)((uint8_t)1 << ((uint8_t)CLK_Peripheral & (uint8_t)0x0F));
      00913C 7B 05            [ 1]  430 	ld	a, (0x05, sp)
      00913E A4 0F            [ 1]  431 	and	a, #0x0f
      009140 88               [ 1]  432 	push	a
      009141 A6 01            [ 1]  433 	ld	a, #0x01
      009143 6B 02            [ 1]  434 	ld	(0x02, sp), a
      009145 84               [ 1]  435 	pop	a
      009146 4D               [ 1]  436 	tnz	a
      009147 27 05            [ 1]  437 	jreq	00276$
      009149                        438 00275$:
      009149 08 01            [ 1]  439 	sll	(0x01, sp)
      00914B 4A               [ 1]  440 	dec	a
      00914C 26 FB            [ 1]  441 	jrne	00275$
      00914E                        442 00276$:
                                    443 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 279: CLK->PCKENR1 &= (uint8_t)(~(uint8_t)(((uint8_t)1 << ((uint8_t)CLK_Peripheral & (uint8_t)0x0F))));
      00914E 7B 01            [ 1]  444 	ld	a, (0x01, sp)
      009150 43               [ 1]  445 	cpl	a
      009151 6B 02            [ 1]  446 	ld	(0x02, sp), a
                                    447 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 269: if (((uint8_t)CLK_Peripheral & (uint8_t)0x10) == 0x00)
      009153 7B 05            [ 1]  448 	ld	a, (0x05, sp)
      009155 A5 10            [ 1]  449 	bcp	a, #0x10
      009157 26 1C            [ 1]  450 	jrne	00108$
                                    451 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 271: if (NewState != DISABLE)
      009159 0D 06            [ 1]  452 	tnz	(0x06, sp)
      00915B 27 0C            [ 1]  453 	jreq	00102$
                                    454 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 274: CLK->PCKENR1 |= (uint8_t)((uint8_t)1 << ((uint8_t)CLK_Peripheral & (uint8_t)0x0F));
      00915D AE 50 C7         [ 2]  455 	ldw	x, #0x50c7
      009160 F6               [ 1]  456 	ld	a, (x)
      009161 1A 01            [ 1]  457 	or	a, (0x01, sp)
      009163 AE 50 C7         [ 2]  458 	ldw	x, #0x50c7
      009166 F7               [ 1]  459 	ld	(x), a
      009167 20 26            [ 2]  460 	jra	00110$
      009169                        461 00102$:
                                    462 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 279: CLK->PCKENR1 &= (uint8_t)(~(uint8_t)(((uint8_t)1 << ((uint8_t)CLK_Peripheral & (uint8_t)0x0F))));
      009169 AE 50 C7         [ 2]  463 	ldw	x, #0x50c7
      00916C F6               [ 1]  464 	ld	a, (x)
      00916D 14 02            [ 1]  465 	and	a, (0x02, sp)
      00916F AE 50 C7         [ 2]  466 	ldw	x, #0x50c7
      009172 F7               [ 1]  467 	ld	(x), a
      009173 20 1A            [ 2]  468 	jra	00110$
      009175                        469 00108$:
                                    470 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 284: if (NewState != DISABLE)
      009175 0D 06            [ 1]  471 	tnz	(0x06, sp)
      009177 27 0C            [ 1]  472 	jreq	00105$
                                    473 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 287: CLK->PCKENR2 |= (uint8_t)((uint8_t)1 << ((uint8_t)CLK_Peripheral & (uint8_t)0x0F));
      009179 AE 50 CA         [ 2]  474 	ldw	x, #0x50ca
      00917C F6               [ 1]  475 	ld	a, (x)
      00917D 1A 01            [ 1]  476 	or	a, (0x01, sp)
      00917F AE 50 CA         [ 2]  477 	ldw	x, #0x50ca
      009182 F7               [ 1]  478 	ld	(x), a
      009183 20 0A            [ 2]  479 	jra	00110$
      009185                        480 00105$:
                                    481 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 292: CLK->PCKENR2 &= (uint8_t)(~(uint8_t)(((uint8_t)1 << ((uint8_t)CLK_Peripheral & (uint8_t)0x0F))));
      009185 AE 50 CA         [ 2]  482 	ldw	x, #0x50ca
      009188 F6               [ 1]  483 	ld	a, (x)
      009189 14 02            [ 1]  484 	and	a, (0x02, sp)
      00918B AE 50 CA         [ 2]  485 	ldw	x, #0x50ca
      00918E F7               [ 1]  486 	ld	(x), a
      00918F                        487 00110$:
      00918F 5B 02            [ 2]  488 	addw	sp, #2
      009191 81               [ 4]  489 	ret
                                    490 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 309: ErrorStatus CLK_ClockSwitchConfig(CLK_SwitchMode_TypeDef CLK_SwitchMode, CLK_Source_TypeDef CLK_NewClock, FunctionalState ITState, CLK_CurrentClockState_TypeDef CLK_CurrentClockState)
                                    491 ;	-----------------------------------------
                                    492 ;	 function CLK_ClockSwitchConfig
                                    493 ;	-----------------------------------------
      009192                        494 _CLK_ClockSwitchConfig:
      009192 52 03            [ 2]  495 	sub	sp, #3
                                    496 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 316: assert_param(IS_CLK_SOURCE_OK(CLK_NewClock));
      009194 7B 07            [ 1]  497 	ld	a, (0x07, sp)
      009196 A1 E1            [ 1]  498 	cp	a, #0xe1
      009198 27 1D            [ 1]  499 	jreq	00140$
      00919A 7B 07            [ 1]  500 	ld	a, (0x07, sp)
      00919C A1 D2            [ 1]  501 	cp	a, #0xd2
      00919E 27 17            [ 1]  502 	jreq	00140$
      0091A0 7B 07            [ 1]  503 	ld	a, (0x07, sp)
      0091A2 A1 B4            [ 1]  504 	cp	a, #0xb4
      0091A4 27 11            [ 1]  505 	jreq	00140$
      0091A6 AE 96 B5         [ 2]  506 	ldw	x, #___str_0+0
      0091A9 4B 3C            [ 1]  507 	push	#0x3c
      0091AB 4B 01            [ 1]  508 	push	#0x01
      0091AD 4B 00            [ 1]  509 	push	#0x00
      0091AF 4B 00            [ 1]  510 	push	#0x00
      0091B1 89               [ 2]  511 	pushw	x
      0091B2 CD 80 A8         [ 4]  512 	call	_assert_failed
      0091B5 5B 06            [ 2]  513 	addw	sp, #6
      0091B7                        514 00140$:
                                    515 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 317: assert_param(IS_CLK_SWITCHMODE_OK(CLK_SwitchMode));
      0091B7 7B 06            [ 1]  516 	ld	a, (0x06, sp)
      0091B9 A1 01            [ 1]  517 	cp	a, #0x01
      0091BB 26 06            [ 1]  518 	jrne	00282$
      0091BD A6 01            [ 1]  519 	ld	a, #0x01
      0091BF 6B 03            [ 1]  520 	ld	(0x03, sp), a
      0091C1 20 02            [ 2]  521 	jra	00283$
      0091C3                        522 00282$:
      0091C3 0F 03            [ 1]  523 	clr	(0x03, sp)
      0091C5                        524 00283$:
      0091C5 0D 06            [ 1]  525 	tnz	(0x06, sp)
      0091C7 27 15            [ 1]  526 	jreq	00148$
      0091C9 0D 03            [ 1]  527 	tnz	(0x03, sp)
      0091CB 26 11            [ 1]  528 	jrne	00148$
      0091CD AE 96 B5         [ 2]  529 	ldw	x, #___str_0+0
      0091D0 4B 3D            [ 1]  530 	push	#0x3d
      0091D2 4B 01            [ 1]  531 	push	#0x01
      0091D4 4B 00            [ 1]  532 	push	#0x00
      0091D6 4B 00            [ 1]  533 	push	#0x00
      0091D8 89               [ 2]  534 	pushw	x
      0091D9 CD 80 A8         [ 4]  535 	call	_assert_failed
      0091DC 5B 06            [ 2]  536 	addw	sp, #6
      0091DE                        537 00148$:
                                    538 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 318: assert_param(IS_FUNCTIONALSTATE_OK(ITState));
      0091DE 0D 08            [ 1]  539 	tnz	(0x08, sp)
      0091E0 27 17            [ 1]  540 	jreq	00153$
      0091E2 7B 08            [ 1]  541 	ld	a, (0x08, sp)
      0091E4 A1 01            [ 1]  542 	cp	a, #0x01
      0091E6 27 11            [ 1]  543 	jreq	00153$
      0091E8 AE 96 B5         [ 2]  544 	ldw	x, #___str_0+0
      0091EB 4B 3E            [ 1]  545 	push	#0x3e
      0091ED 4B 01            [ 1]  546 	push	#0x01
      0091EF 4B 00            [ 1]  547 	push	#0x00
      0091F1 4B 00            [ 1]  548 	push	#0x00
      0091F3 89               [ 2]  549 	pushw	x
      0091F4 CD 80 A8         [ 4]  550 	call	_assert_failed
      0091F7 5B 06            [ 2]  551 	addw	sp, #6
      0091F9                        552 00153$:
                                    553 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 319: assert_param(IS_CLK_CURRENTCLOCKSTATE_OK(CLK_CurrentClockState));
      0091F9 0D 09            [ 1]  554 	tnz	(0x09, sp)
      0091FB 27 17            [ 1]  555 	jreq	00158$
      0091FD 7B 09            [ 1]  556 	ld	a, (0x09, sp)
      0091FF A1 01            [ 1]  557 	cp	a, #0x01
      009201 27 11            [ 1]  558 	jreq	00158$
      009203 AE 96 B5         [ 2]  559 	ldw	x, #___str_0+0
      009206 4B 3F            [ 1]  560 	push	#0x3f
      009208 4B 01            [ 1]  561 	push	#0x01
      00920A 4B 00            [ 1]  562 	push	#0x00
      00920C 4B 00            [ 1]  563 	push	#0x00
      00920E 89               [ 2]  564 	pushw	x
      00920F CD 80 A8         [ 4]  565 	call	_assert_failed
      009212 5B 06            [ 2]  566 	addw	sp, #6
      009214                        567 00158$:
                                    568 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 322: clock_master = (CLK_Source_TypeDef)CLK->CMSR;
      009214 AE 50 C3         [ 2]  569 	ldw	x, #0x50c3
      009217 F6               [ 1]  570 	ld	a, (x)
      009218 6B 02            [ 1]  571 	ld	(0x02, sp), a
                                    572 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 325: if (CLK_SwitchMode == CLK_SWITCHMODE_AUTO)
      00921A 0D 03            [ 1]  573 	tnz	(0x03, sp)
      00921C 27 40            [ 1]  574 	jreq	00122$
                                    575 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 328: CLK->SWCR |= CLK_SWCR_SWEN;
      00921E AE 50 C5         [ 2]  576 	ldw	x, #0x50c5
      009221 F6               [ 1]  577 	ld	a, (x)
      009222 AA 02            [ 1]  578 	or	a, #0x02
      009224 F7               [ 1]  579 	ld	(x), a
                                    580 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 331: if (ITState != DISABLE)
      009225 0D 08            [ 1]  581 	tnz	(0x08, sp)
      009227 27 09            [ 1]  582 	jreq	00102$
                                    583 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 333: CLK->SWCR |= CLK_SWCR_SWIEN;
      009229 AE 50 C5         [ 2]  584 	ldw	x, #0x50c5
      00922C F6               [ 1]  585 	ld	a, (x)
      00922D AA 04            [ 1]  586 	or	a, #0x04
      00922F F7               [ 1]  587 	ld	(x), a
      009230 20 07            [ 2]  588 	jra	00103$
      009232                        589 00102$:
                                    590 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 337: CLK->SWCR &= (uint8_t)(~CLK_SWCR_SWIEN);
      009232 AE 50 C5         [ 2]  591 	ldw	x, #0x50c5
      009235 F6               [ 1]  592 	ld	a, (x)
      009236 A4 FB            [ 1]  593 	and	a, #0xfb
      009238 F7               [ 1]  594 	ld	(x), a
      009239                        595 00103$:
                                    596 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 341: CLK->SWR = (uint8_t)CLK_NewClock;
      009239 AE 50 C4         [ 2]  597 	ldw	x, #0x50c4
      00923C 7B 07            [ 1]  598 	ld	a, (0x07, sp)
      00923E F7               [ 1]  599 	ld	(x), a
                                    600 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 344: while((((CLK->SWCR & CLK_SWCR_SWBSY) != 0 )&& (DownCounter != 0)))
      00923F AE FF FF         [ 2]  601 	ldw	x, #0xffff
      009242                        602 00105$:
      009242 90 AE 50 C5      [ 2]  603 	ldw	y, #0x50c5
      009246 90 F6            [ 1]  604 	ld	a, (y)
      009248 44               [ 1]  605 	srl	a
      009249 24 06            [ 1]  606 	jrnc	00107$
      00924B 5D               [ 2]  607 	tnzw	x
      00924C 27 03            [ 1]  608 	jreq	00107$
                                    609 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 346: DownCounter--;
      00924E 5A               [ 2]  610 	decw	x
      00924F 20 F1            [ 2]  611 	jra	00105$
      009251                        612 00107$:
                                    613 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 349: if(DownCounter != 0)
      009251 5D               [ 2]  614 	tnzw	x
      009252 27 06            [ 1]  615 	jreq	00109$
                                    616 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 351: Swif = SUCCESS;
      009254 A6 01            [ 1]  617 	ld	a, #0x01
      009256 6B 01            [ 1]  618 	ld	(0x01, sp), a
      009258 20 43            [ 2]  619 	jra	00123$
      00925A                        620 00109$:
                                    621 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 355: Swif = ERROR;
      00925A 0F 01            [ 1]  622 	clr	(0x01, sp)
      00925C 20 3F            [ 2]  623 	jra	00123$
      00925E                        624 00122$:
                                    625 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 361: if (ITState != DISABLE)
      00925E 0D 08            [ 1]  626 	tnz	(0x08, sp)
      009260 27 09            [ 1]  627 	jreq	00112$
                                    628 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 363: CLK->SWCR |= CLK_SWCR_SWIEN;
      009262 AE 50 C5         [ 2]  629 	ldw	x, #0x50c5
      009265 F6               [ 1]  630 	ld	a, (x)
      009266 AA 04            [ 1]  631 	or	a, #0x04
      009268 F7               [ 1]  632 	ld	(x), a
      009269 20 07            [ 2]  633 	jra	00113$
      00926B                        634 00112$:
                                    635 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 367: CLK->SWCR &= (uint8_t)(~CLK_SWCR_SWIEN);
      00926B AE 50 C5         [ 2]  636 	ldw	x, #0x50c5
      00926E F6               [ 1]  637 	ld	a, (x)
      00926F A4 FB            [ 1]  638 	and	a, #0xfb
      009271 F7               [ 1]  639 	ld	(x), a
      009272                        640 00113$:
                                    641 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 371: CLK->SWR = (uint8_t)CLK_NewClock;
      009272 AE 50 C4         [ 2]  642 	ldw	x, #0x50c4
      009275 7B 07            [ 1]  643 	ld	a, (0x07, sp)
      009277 F7               [ 1]  644 	ld	(x), a
                                    645 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 374: while((((CLK->SWCR & CLK_SWCR_SWIF) != 0 ) && (DownCounter != 0)))
      009278 AE FF FF         [ 2]  646 	ldw	x, #0xffff
      00927B                        647 00115$:
      00927B 90 AE 50 C5      [ 2]  648 	ldw	y, #0x50c5
      00927F 90 F6            [ 1]  649 	ld	a, (y)
      009281 A5 08            [ 1]  650 	bcp	a, #0x08
      009283 27 06            [ 1]  651 	jreq	00117$
      009285 5D               [ 2]  652 	tnzw	x
      009286 27 03            [ 1]  653 	jreq	00117$
                                    654 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 376: DownCounter--;
      009288 5A               [ 2]  655 	decw	x
      009289 20 F0            [ 2]  656 	jra	00115$
      00928B                        657 00117$:
                                    658 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 379: if(DownCounter != 0)
      00928B 5D               [ 2]  659 	tnzw	x
      00928C 27 0D            [ 1]  660 	jreq	00119$
                                    661 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 382: CLK->SWCR |= CLK_SWCR_SWEN;
      00928E AE 50 C5         [ 2]  662 	ldw	x, #0x50c5
      009291 F6               [ 1]  663 	ld	a, (x)
      009292 AA 02            [ 1]  664 	or	a, #0x02
      009294 F7               [ 1]  665 	ld	(x), a
                                    666 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 383: Swif = SUCCESS;
      009295 A6 01            [ 1]  667 	ld	a, #0x01
      009297 6B 01            [ 1]  668 	ld	(0x01, sp), a
      009299 20 02            [ 2]  669 	jra	00123$
      00929B                        670 00119$:
                                    671 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 387: Swif = ERROR;
      00929B 0F 01            [ 1]  672 	clr	(0x01, sp)
      00929D                        673 00123$:
                                    674 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 390: if(Swif != ERROR)
      00929D 0D 01            [ 1]  675 	tnz	(0x01, sp)
      00929F 27 37            [ 1]  676 	jreq	00136$
                                    677 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 393: if((CLK_CurrentClockState == CLK_CURRENTCLOCKSTATE_DISABLE) && ( clock_master == CLK_SOURCE_HSI))
      0092A1 0D 09            [ 1]  678 	tnz	(0x09, sp)
      0092A3 26 0F            [ 1]  679 	jrne	00132$
      0092A5 7B 02            [ 1]  680 	ld	a, (0x02, sp)
      0092A7 A1 E1            [ 1]  681 	cp	a, #0xe1
      0092A9 26 09            [ 1]  682 	jrne	00132$
                                    683 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 395: CLK->ICKR &= (uint8_t)(~CLK_ICKR_HSIEN);
      0092AB AE 50 C0         [ 2]  684 	ldw	x, #0x50c0
      0092AE F6               [ 1]  685 	ld	a, (x)
      0092AF A4 FE            [ 1]  686 	and	a, #0xfe
      0092B1 F7               [ 1]  687 	ld	(x), a
      0092B2 20 24            [ 2]  688 	jra	00136$
      0092B4                        689 00132$:
                                    690 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 397: else if((CLK_CurrentClockState == CLK_CURRENTCLOCKSTATE_DISABLE) && ( clock_master == CLK_SOURCE_LSI))
      0092B4 0D 09            [ 1]  691 	tnz	(0x09, sp)
      0092B6 26 0F            [ 1]  692 	jrne	00128$
      0092B8 7B 02            [ 1]  693 	ld	a, (0x02, sp)
      0092BA A1 D2            [ 1]  694 	cp	a, #0xd2
      0092BC 26 09            [ 1]  695 	jrne	00128$
                                    696 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 399: CLK->ICKR &= (uint8_t)(~CLK_ICKR_LSIEN);
      0092BE AE 50 C0         [ 2]  697 	ldw	x, #0x50c0
      0092C1 F6               [ 1]  698 	ld	a, (x)
      0092C2 A4 F7            [ 1]  699 	and	a, #0xf7
      0092C4 F7               [ 1]  700 	ld	(x), a
      0092C5 20 11            [ 2]  701 	jra	00136$
      0092C7                        702 00128$:
                                    703 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 401: else if ((CLK_CurrentClockState == CLK_CURRENTCLOCKSTATE_DISABLE) && ( clock_master == CLK_SOURCE_HSE))
      0092C7 0D 09            [ 1]  704 	tnz	(0x09, sp)
      0092C9 26 0D            [ 1]  705 	jrne	00136$
      0092CB 7B 02            [ 1]  706 	ld	a, (0x02, sp)
      0092CD A1 B4            [ 1]  707 	cp	a, #0xb4
      0092CF 26 07            [ 1]  708 	jrne	00136$
                                    709 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 403: CLK->ECKR &= (uint8_t)(~CLK_ECKR_HSEEN);
      0092D1 AE 50 C1         [ 2]  710 	ldw	x, #0x50c1
      0092D4 F6               [ 1]  711 	ld	a, (x)
      0092D5 A4 FE            [ 1]  712 	and	a, #0xfe
      0092D7 F7               [ 1]  713 	ld	(x), a
      0092D8                        714 00136$:
                                    715 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 406: return(Swif);
      0092D8 7B 01            [ 1]  716 	ld	a, (0x01, sp)
      0092DA 5B 03            [ 2]  717 	addw	sp, #3
      0092DC 81               [ 4]  718 	ret
                                    719 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 415: void CLK_HSIPrescalerConfig(CLK_Prescaler_TypeDef HSIPrescaler)
                                    720 ;	-----------------------------------------
                                    721 ;	 function CLK_HSIPrescalerConfig
                                    722 ;	-----------------------------------------
      0092DD                        723 _CLK_HSIPrescalerConfig:
                                    724 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 418: assert_param(IS_CLK_HSIPRESCALER_OK(HSIPrescaler));
      0092DD 0D 03            [ 1]  725 	tnz	(0x03, sp)
      0092DF 27 23            [ 1]  726 	jreq	00104$
      0092E1 7B 03            [ 1]  727 	ld	a, (0x03, sp)
      0092E3 A1 08            [ 1]  728 	cp	a, #0x08
      0092E5 27 1D            [ 1]  729 	jreq	00104$
      0092E7 7B 03            [ 1]  730 	ld	a, (0x03, sp)
      0092E9 A1 10            [ 1]  731 	cp	a, #0x10
      0092EB 27 17            [ 1]  732 	jreq	00104$
      0092ED 7B 03            [ 1]  733 	ld	a, (0x03, sp)
      0092EF A1 18            [ 1]  734 	cp	a, #0x18
      0092F1 27 11            [ 1]  735 	jreq	00104$
      0092F3 AE 96 B5         [ 2]  736 	ldw	x, #___str_0+0
      0092F6 4B A2            [ 1]  737 	push	#0xa2
      0092F8 4B 01            [ 1]  738 	push	#0x01
      0092FA 4B 00            [ 1]  739 	push	#0x00
      0092FC 4B 00            [ 1]  740 	push	#0x00
      0092FE 89               [ 2]  741 	pushw	x
      0092FF CD 80 A8         [ 4]  742 	call	_assert_failed
      009302 5B 06            [ 2]  743 	addw	sp, #6
      009304                        744 00104$:
                                    745 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 421: CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_HSIDIV);
      009304 AE 50 C6         [ 2]  746 	ldw	x, #0x50c6
      009307 F6               [ 1]  747 	ld	a, (x)
      009308 A4 E7            [ 1]  748 	and	a, #0xe7
      00930A F7               [ 1]  749 	ld	(x), a
                                    750 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 424: CLK->CKDIVR |= (uint8_t)HSIPrescaler;
      00930B AE 50 C6         [ 2]  751 	ldw	x, #0x50c6
      00930E F6               [ 1]  752 	ld	a, (x)
      00930F 1A 03            [ 1]  753 	or	a, (0x03, sp)
      009311 AE 50 C6         [ 2]  754 	ldw	x, #0x50c6
      009314 F7               [ 1]  755 	ld	(x), a
      009315 81               [ 4]  756 	ret
                                    757 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 436: void CLK_CCOConfig(CLK_Output_TypeDef CLK_CCO)
                                    758 ;	-----------------------------------------
                                    759 ;	 function CLK_CCOConfig
                                    760 ;	-----------------------------------------
      009316                        761 _CLK_CCOConfig:
                                    762 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 439: assert_param(IS_CLK_OUTPUT_OK(CLK_CCO));
      009316 0D 03            [ 1]  763 	tnz	(0x03, sp)
      009318 27 59            [ 1]  764 	jreq	00104$
      00931A 7B 03            [ 1]  765 	ld	a, (0x03, sp)
      00931C A1 04            [ 1]  766 	cp	a, #0x04
      00931E 27 53            [ 1]  767 	jreq	00104$
      009320 7B 03            [ 1]  768 	ld	a, (0x03, sp)
      009322 A1 02            [ 1]  769 	cp	a, #0x02
      009324 27 4D            [ 1]  770 	jreq	00104$
      009326 7B 03            [ 1]  771 	ld	a, (0x03, sp)
      009328 A1 08            [ 1]  772 	cp	a, #0x08
      00932A 27 47            [ 1]  773 	jreq	00104$
      00932C 7B 03            [ 1]  774 	ld	a, (0x03, sp)
      00932E A1 0A            [ 1]  775 	cp	a, #0x0a
      009330 27 41            [ 1]  776 	jreq	00104$
      009332 7B 03            [ 1]  777 	ld	a, (0x03, sp)
      009334 A1 0C            [ 1]  778 	cp	a, #0x0c
      009336 27 3B            [ 1]  779 	jreq	00104$
      009338 7B 03            [ 1]  780 	ld	a, (0x03, sp)
      00933A A1 0E            [ 1]  781 	cp	a, #0x0e
      00933C 27 35            [ 1]  782 	jreq	00104$
      00933E 7B 03            [ 1]  783 	ld	a, (0x03, sp)
      009340 A1 10            [ 1]  784 	cp	a, #0x10
      009342 27 2F            [ 1]  785 	jreq	00104$
      009344 7B 03            [ 1]  786 	ld	a, (0x03, sp)
      009346 A1 12            [ 1]  787 	cp	a, #0x12
      009348 27 29            [ 1]  788 	jreq	00104$
      00934A 7B 03            [ 1]  789 	ld	a, (0x03, sp)
      00934C A1 14            [ 1]  790 	cp	a, #0x14
      00934E 27 23            [ 1]  791 	jreq	00104$
      009350 7B 03            [ 1]  792 	ld	a, (0x03, sp)
      009352 A1 16            [ 1]  793 	cp	a, #0x16
      009354 27 1D            [ 1]  794 	jreq	00104$
      009356 7B 03            [ 1]  795 	ld	a, (0x03, sp)
      009358 A1 18            [ 1]  796 	cp	a, #0x18
      00935A 27 17            [ 1]  797 	jreq	00104$
      00935C 7B 03            [ 1]  798 	ld	a, (0x03, sp)
      00935E A1 1A            [ 1]  799 	cp	a, #0x1a
      009360 27 11            [ 1]  800 	jreq	00104$
      009362 AE 96 B5         [ 2]  801 	ldw	x, #___str_0+0
      009365 4B B7            [ 1]  802 	push	#0xb7
      009367 4B 01            [ 1]  803 	push	#0x01
      009369 4B 00            [ 1]  804 	push	#0x00
      00936B 4B 00            [ 1]  805 	push	#0x00
      00936D 89               [ 2]  806 	pushw	x
      00936E CD 80 A8         [ 4]  807 	call	_assert_failed
      009371 5B 06            [ 2]  808 	addw	sp, #6
      009373                        809 00104$:
                                    810 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 442: CLK->CCOR &= (uint8_t)(~CLK_CCOR_CCOSEL);
      009373 AE 50 C9         [ 2]  811 	ldw	x, #0x50c9
      009376 F6               [ 1]  812 	ld	a, (x)
      009377 A4 E1            [ 1]  813 	and	a, #0xe1
      009379 F7               [ 1]  814 	ld	(x), a
                                    815 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 445: CLK->CCOR |= (uint8_t)CLK_CCO;
      00937A AE 50 C9         [ 2]  816 	ldw	x, #0x50c9
      00937D F6               [ 1]  817 	ld	a, (x)
      00937E 1A 03            [ 1]  818 	or	a, (0x03, sp)
      009380 AE 50 C9         [ 2]  819 	ldw	x, #0x50c9
      009383 F7               [ 1]  820 	ld	(x), a
                                    821 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 448: CLK->CCOR |= CLK_CCOR_CCOEN;
      009384 72 10 50 C9      [ 1]  822 	bset	0x50c9, #0
      009388 81               [ 4]  823 	ret
                                    824 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 459: void CLK_ITConfig(CLK_IT_TypeDef CLK_IT, FunctionalState NewState)
                                    825 ;	-----------------------------------------
                                    826 ;	 function CLK_ITConfig
                                    827 ;	-----------------------------------------
      009389                        828 _CLK_ITConfig:
      009389 52 02            [ 2]  829 	sub	sp, #2
                                    830 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 462: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      00938B 0D 06            [ 1]  831 	tnz	(0x06, sp)
      00938D 27 17            [ 1]  832 	jreq	00115$
      00938F 7B 06            [ 1]  833 	ld	a, (0x06, sp)
      009391 A1 01            [ 1]  834 	cp	a, #0x01
      009393 27 11            [ 1]  835 	jreq	00115$
      009395 AE 96 B5         [ 2]  836 	ldw	x, #___str_0+0
      009398 4B CE            [ 1]  837 	push	#0xce
      00939A 4B 01            [ 1]  838 	push	#0x01
      00939C 4B 00            [ 1]  839 	push	#0x00
      00939E 4B 00            [ 1]  840 	push	#0x00
      0093A0 89               [ 2]  841 	pushw	x
      0093A1 CD 80 A8         [ 4]  842 	call	_assert_failed
      0093A4 5B 06            [ 2]  843 	addw	sp, #6
      0093A6                        844 00115$:
                                    845 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 463: assert_param(IS_CLK_IT_OK(CLK_IT));
      0093A6 7B 05            [ 1]  846 	ld	a, (0x05, sp)
      0093A8 A1 0C            [ 1]  847 	cp	a, #0x0c
      0093AA 26 06            [ 1]  848 	jrne	00165$
      0093AC A6 01            [ 1]  849 	ld	a, #0x01
      0093AE 6B 02            [ 1]  850 	ld	(0x02, sp), a
      0093B0 20 02            [ 2]  851 	jra	00166$
      0093B2                        852 00165$:
      0093B2 0F 02            [ 1]  853 	clr	(0x02, sp)
      0093B4                        854 00166$:
      0093B4 7B 05            [ 1]  855 	ld	a, (0x05, sp)
      0093B6 A1 1C            [ 1]  856 	cp	a, #0x1c
      0093B8 26 06            [ 1]  857 	jrne	00168$
      0093BA A6 01            [ 1]  858 	ld	a, #0x01
      0093BC 6B 01            [ 1]  859 	ld	(0x01, sp), a
      0093BE 20 02            [ 2]  860 	jra	00169$
      0093C0                        861 00168$:
      0093C0 0F 01            [ 1]  862 	clr	(0x01, sp)
      0093C2                        863 00169$:
      0093C2 0D 02            [ 1]  864 	tnz	(0x02, sp)
      0093C4 26 15            [ 1]  865 	jrne	00120$
      0093C6 0D 01            [ 1]  866 	tnz	(0x01, sp)
      0093C8 26 11            [ 1]  867 	jrne	00120$
      0093CA AE 96 B5         [ 2]  868 	ldw	x, #___str_0+0
      0093CD 4B CF            [ 1]  869 	push	#0xcf
      0093CF 4B 01            [ 1]  870 	push	#0x01
      0093D1 4B 00            [ 1]  871 	push	#0x00
      0093D3 4B 00            [ 1]  872 	push	#0x00
      0093D5 89               [ 2]  873 	pushw	x
      0093D6 CD 80 A8         [ 4]  874 	call	_assert_failed
      0093D9 5B 06            [ 2]  875 	addw	sp, #6
      0093DB                        876 00120$:
                                    877 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 465: if (NewState != DISABLE)
      0093DB 0D 06            [ 1]  878 	tnz	(0x06, sp)
      0093DD 27 1A            [ 1]  879 	jreq	00110$
                                    880 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 467: switch (CLK_IT)
      0093DF 0D 02            [ 1]  881 	tnz	(0x02, sp)
      0093E1 26 0D            [ 1]  882 	jrne	00102$
      0093E3 0D 01            [ 1]  883 	tnz	(0x01, sp)
      0093E5 27 2A            [ 1]  884 	jreq	00112$
                                    885 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 470: CLK->SWCR |= CLK_SWCR_SWIEN;
      0093E7 AE 50 C5         [ 2]  886 	ldw	x, #0x50c5
      0093EA F6               [ 1]  887 	ld	a, (x)
      0093EB AA 04            [ 1]  888 	or	a, #0x04
      0093ED F7               [ 1]  889 	ld	(x), a
                                    890 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 471: break;
      0093EE 20 21            [ 2]  891 	jra	00112$
                                    892 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 472: case CLK_IT_CSSD: /* Enable the clock security system detection interrupt */
      0093F0                        893 00102$:
                                    894 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 473: CLK->CSSR |= CLK_CSSR_CSSDIE;
      0093F0 AE 50 C8         [ 2]  895 	ldw	x, #0x50c8
      0093F3 F6               [ 1]  896 	ld	a, (x)
      0093F4 AA 04            [ 1]  897 	or	a, #0x04
      0093F6 F7               [ 1]  898 	ld	(x), a
                                    899 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 474: break;
      0093F7 20 18            [ 2]  900 	jra	00112$
                                    901 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 477: }
      0093F9                        902 00110$:
                                    903 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 481: switch (CLK_IT)
      0093F9 0D 02            [ 1]  904 	tnz	(0x02, sp)
      0093FB 26 0D            [ 1]  905 	jrne	00106$
      0093FD 0D 01            [ 1]  906 	tnz	(0x01, sp)
      0093FF 27 10            [ 1]  907 	jreq	00112$
                                    908 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 484: CLK->SWCR  &= (uint8_t)(~CLK_SWCR_SWIEN);
      009401 AE 50 C5         [ 2]  909 	ldw	x, #0x50c5
      009404 F6               [ 1]  910 	ld	a, (x)
      009405 A4 FB            [ 1]  911 	and	a, #0xfb
      009407 F7               [ 1]  912 	ld	(x), a
                                    913 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 485: break;
      009408 20 07            [ 2]  914 	jra	00112$
                                    915 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 486: case CLK_IT_CSSD: /* Disable the clock security system detection interrupt */
      00940A                        916 00106$:
                                    917 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 487: CLK->CSSR &= (uint8_t)(~CLK_CSSR_CSSDIE);
      00940A AE 50 C8         [ 2]  918 	ldw	x, #0x50c8
      00940D F6               [ 1]  919 	ld	a, (x)
      00940E A4 FB            [ 1]  920 	and	a, #0xfb
      009410 F7               [ 1]  921 	ld	(x), a
                                    922 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 491: }
      009411                        923 00112$:
      009411 5B 02            [ 2]  924 	addw	sp, #2
      009413 81               [ 4]  925 	ret
                                    926 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 500: void CLK_SYSCLKConfig(CLK_Prescaler_TypeDef CLK_Prescaler)
                                    927 ;	-----------------------------------------
                                    928 ;	 function CLK_SYSCLKConfig
                                    929 ;	-----------------------------------------
      009414                        930 _CLK_SYSCLKConfig:
      009414 52 02            [ 2]  931 	sub	sp, #2
                                    932 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 503: assert_param(IS_CLK_PRESCALER_OK(CLK_Prescaler));
      009416 0D 05            [ 1]  933 	tnz	(0x05, sp)
      009418 27 53            [ 1]  934 	jreq	00107$
      00941A 7B 05            [ 1]  935 	ld	a, (0x05, sp)
      00941C A1 08            [ 1]  936 	cp	a, #0x08
      00941E 27 4D            [ 1]  937 	jreq	00107$
      009420 7B 05            [ 1]  938 	ld	a, (0x05, sp)
      009422 A1 10            [ 1]  939 	cp	a, #0x10
      009424 27 47            [ 1]  940 	jreq	00107$
      009426 7B 05            [ 1]  941 	ld	a, (0x05, sp)
      009428 A1 18            [ 1]  942 	cp	a, #0x18
      00942A 27 41            [ 1]  943 	jreq	00107$
      00942C 7B 05            [ 1]  944 	ld	a, (0x05, sp)
      00942E A1 80            [ 1]  945 	cp	a, #0x80
      009430 27 3B            [ 1]  946 	jreq	00107$
      009432 7B 05            [ 1]  947 	ld	a, (0x05, sp)
      009434 A1 81            [ 1]  948 	cp	a, #0x81
      009436 27 35            [ 1]  949 	jreq	00107$
      009438 7B 05            [ 1]  950 	ld	a, (0x05, sp)
      00943A A1 82            [ 1]  951 	cp	a, #0x82
      00943C 27 2F            [ 1]  952 	jreq	00107$
      00943E 7B 05            [ 1]  953 	ld	a, (0x05, sp)
      009440 A1 83            [ 1]  954 	cp	a, #0x83
      009442 27 29            [ 1]  955 	jreq	00107$
      009444 7B 05            [ 1]  956 	ld	a, (0x05, sp)
      009446 A1 84            [ 1]  957 	cp	a, #0x84
      009448 27 23            [ 1]  958 	jreq	00107$
      00944A 7B 05            [ 1]  959 	ld	a, (0x05, sp)
      00944C A1 85            [ 1]  960 	cp	a, #0x85
      00944E 27 1D            [ 1]  961 	jreq	00107$
      009450 7B 05            [ 1]  962 	ld	a, (0x05, sp)
      009452 A1 86            [ 1]  963 	cp	a, #0x86
      009454 27 17            [ 1]  964 	jreq	00107$
      009456 7B 05            [ 1]  965 	ld	a, (0x05, sp)
      009458 A1 87            [ 1]  966 	cp	a, #0x87
      00945A 27 11            [ 1]  967 	jreq	00107$
      00945C AE 96 B5         [ 2]  968 	ldw	x, #___str_0+0
      00945F 4B F7            [ 1]  969 	push	#0xf7
      009461 4B 01            [ 1]  970 	push	#0x01
      009463 4B 00            [ 1]  971 	push	#0x00
      009465 4B 00            [ 1]  972 	push	#0x00
      009467 89               [ 2]  973 	pushw	x
      009468 CD 80 A8         [ 4]  974 	call	_assert_failed
      00946B 5B 06            [ 2]  975 	addw	sp, #6
      00946D                        976 00107$:
                                    977 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 505: if (((uint8_t)CLK_Prescaler & (uint8_t)0x80) == 0x00) /* Bit7 = 0 means HSI divider */
      00946D 0D 05            [ 1]  978 	tnz	(0x05, sp)
      00946F 2B 19            [ 1]  979 	jrmi	00102$
                                    980 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 507: CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_HSIDIV);
      009471 AE 50 C6         [ 2]  981 	ldw	x, #0x50c6
      009474 F6               [ 1]  982 	ld	a, (x)
      009475 A4 E7            [ 1]  983 	and	a, #0xe7
      009477 F7               [ 1]  984 	ld	(x), a
                                    985 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 508: CLK->CKDIVR |= (uint8_t)((uint8_t)CLK_Prescaler & (uint8_t)CLK_CKDIVR_HSIDIV);
      009478 AE 50 C6         [ 2]  986 	ldw	x, #0x50c6
      00947B F6               [ 1]  987 	ld	a, (x)
      00947C 6B 01            [ 1]  988 	ld	(0x01, sp), a
      00947E 7B 05            [ 1]  989 	ld	a, (0x05, sp)
      009480 A4 18            [ 1]  990 	and	a, #0x18
      009482 1A 01            [ 1]  991 	or	a, (0x01, sp)
      009484 AE 50 C6         [ 2]  992 	ldw	x, #0x50c6
      009487 F7               [ 1]  993 	ld	(x), a
      009488 20 17            [ 2]  994 	jra	00104$
      00948A                        995 00102$:
                                    996 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 512: CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_CPUDIV);
      00948A AE 50 C6         [ 2]  997 	ldw	x, #0x50c6
      00948D F6               [ 1]  998 	ld	a, (x)
      00948E A4 F8            [ 1]  999 	and	a, #0xf8
      009490 F7               [ 1] 1000 	ld	(x), a
                                   1001 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 513: CLK->CKDIVR |= (uint8_t)((uint8_t)CLK_Prescaler & (uint8_t)CLK_CKDIVR_CPUDIV);
      009491 AE 50 C6         [ 2] 1002 	ldw	x, #0x50c6
      009494 F6               [ 1] 1003 	ld	a, (x)
      009495 6B 02            [ 1] 1004 	ld	(0x02, sp), a
      009497 7B 05            [ 1] 1005 	ld	a, (0x05, sp)
      009499 A4 07            [ 1] 1006 	and	a, #0x07
      00949B 1A 02            [ 1] 1007 	or	a, (0x02, sp)
      00949D AE 50 C6         [ 2] 1008 	ldw	x, #0x50c6
      0094A0 F7               [ 1] 1009 	ld	(x), a
      0094A1                       1010 00104$:
      0094A1 5B 02            [ 2] 1011 	addw	sp, #2
      0094A3 81               [ 4] 1012 	ret
                                   1013 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 523: void CLK_SWIMConfig(CLK_SWIMDivider_TypeDef CLK_SWIMDivider)
                                   1014 ;	-----------------------------------------
                                   1015 ;	 function CLK_SWIMConfig
                                   1016 ;	-----------------------------------------
      0094A4                       1017 _CLK_SWIMConfig:
                                   1018 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 526: assert_param(IS_CLK_SWIMDIVIDER_OK(CLK_SWIMDivider));
      0094A4 0D 03            [ 1] 1019 	tnz	(0x03, sp)
      0094A6 27 17            [ 1] 1020 	jreq	00107$
      0094A8 7B 03            [ 1] 1021 	ld	a, (0x03, sp)
      0094AA A1 01            [ 1] 1022 	cp	a, #0x01
      0094AC 27 11            [ 1] 1023 	jreq	00107$
      0094AE AE 96 B5         [ 2] 1024 	ldw	x, #___str_0+0
      0094B1 4B 0E            [ 1] 1025 	push	#0x0e
      0094B3 4B 02            [ 1] 1026 	push	#0x02
      0094B5 4B 00            [ 1] 1027 	push	#0x00
      0094B7 4B 00            [ 1] 1028 	push	#0x00
      0094B9 89               [ 2] 1029 	pushw	x
      0094BA CD 80 A8         [ 4] 1030 	call	_assert_failed
      0094BD 5B 06            [ 2] 1031 	addw	sp, #6
      0094BF                       1032 00107$:
                                   1033 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 528: if (CLK_SWIMDivider != CLK_SWIMDIVIDER_2)
      0094BF 0D 03            [ 1] 1034 	tnz	(0x03, sp)
      0094C1 27 05            [ 1] 1035 	jreq	00102$
                                   1036 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 531: CLK->SWIMCCR |= CLK_SWIMCCR_SWIMDIV;
      0094C3 72 10 50 CD      [ 1] 1037 	bset	0x50cd, #0
      0094C7 81               [ 4] 1038 	ret
      0094C8                       1039 00102$:
                                   1040 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 536: CLK->SWIMCCR &= (uint8_t)(~CLK_SWIMCCR_SWIMDIV);
      0094C8 72 11 50 CD      [ 1] 1041 	bres	0x50cd, #0
      0094CC 81               [ 4] 1042 	ret
                                   1043 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 547: void CLK_ClockSecuritySystemEnable(void)
                                   1044 ;	-----------------------------------------
                                   1045 ;	 function CLK_ClockSecuritySystemEnable
                                   1046 ;	-----------------------------------------
      0094CD                       1047 _CLK_ClockSecuritySystemEnable:
                                   1048 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 550: CLK->CSSR |= CLK_CSSR_CSSEN;
      0094CD 72 10 50 C8      [ 1] 1049 	bset	0x50c8, #0
      0094D1 81               [ 4] 1050 	ret
                                   1051 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 559: CLK_Source_TypeDef CLK_GetSYSCLKSource(void)
                                   1052 ;	-----------------------------------------
                                   1053 ;	 function CLK_GetSYSCLKSource
                                   1054 ;	-----------------------------------------
      0094D2                       1055 _CLK_GetSYSCLKSource:
                                   1056 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 561: return((CLK_Source_TypeDef)CLK->CMSR);
      0094D2 AE 50 C3         [ 2] 1057 	ldw	x, #0x50c3
      0094D5 F6               [ 1] 1058 	ld	a, (x)
      0094D6 81               [ 4] 1059 	ret
                                   1060 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 569: uint32_t CLK_GetClockFreq(void)
                                   1061 ;	-----------------------------------------
                                   1062 ;	 function CLK_GetClockFreq
                                   1063 ;	-----------------------------------------
      0094D7                       1064 _CLK_GetClockFreq:
      0094D7 52 07            [ 2] 1065 	sub	sp, #7
                                   1066 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 576: clocksource = (CLK_Source_TypeDef)CLK->CMSR;
      0094D9 AE 50 C3         [ 2] 1067 	ldw	x, #0x50c3
      0094DC F6               [ 1] 1068 	ld	a, (x)
      0094DD 6B 01            [ 1] 1069 	ld	(0x01, sp), a
                                   1070 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 578: if (clocksource == CLK_SOURCE_HSI)
      0094DF 7B 01            [ 1] 1071 	ld	a, (0x01, sp)
      0094E1 A1 E1            [ 1] 1072 	cp	a, #0xe1
      0094E3 26 2C            [ 1] 1073 	jrne	00105$
                                   1074 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 580: tmp = (uint8_t)(CLK->CKDIVR & CLK_CKDIVR_HSIDIV);
      0094E5 AE 50 C6         [ 2] 1075 	ldw	x, #0x50c6
      0094E8 F6               [ 1] 1076 	ld	a, (x)
      0094E9 A4 18            [ 1] 1077 	and	a, #0x18
                                   1078 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 581: tmp = (uint8_t)(tmp >> 3);
      0094EB 44               [ 1] 1079 	srl	a
      0094EC 44               [ 1] 1080 	srl	a
      0094ED 44               [ 1] 1081 	srl	a
                                   1082 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 582: presc = HSIDivFactor[tmp];
      0094EE AE 96 A9         [ 2] 1083 	ldw	x, #_HSIDivFactor+0
      0094F1 1F 06            [ 2] 1084 	ldw	(0x06, sp), x
      0094F3 5F               [ 1] 1085 	clrw	x
      0094F4 97               [ 1] 1086 	ld	xl, a
      0094F5 72 FB 06         [ 2] 1087 	addw	x, (0x06, sp)
      0094F8 F6               [ 1] 1088 	ld	a, (x)
                                   1089 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 583: clockfrequency = HSI_VALUE / presc;
      0094F9 5F               [ 1] 1090 	clrw	x
      0094FA 97               [ 1] 1091 	ld	xl, a
      0094FB 90 5F            [ 1] 1092 	clrw	y
      0094FD 89               [ 2] 1093 	pushw	x
      0094FE 90 89            [ 2] 1094 	pushw	y
      009500 4B 00            [ 1] 1095 	push	#0x00
      009502 4B 24            [ 1] 1096 	push	#0x24
      009504 4B F4            [ 1] 1097 	push	#0xf4
      009506 4B 00            [ 1] 1098 	push	#0x00
      009508 CD 98 6A         [ 4] 1099 	call	__divulong
      00950B 5B 08            [ 2] 1100 	addw	sp, #8
      00950D 1F 04            [ 2] 1101 	ldw	(0x04, sp), x
      00950F 20 1A            [ 2] 1102 	jra	00106$
      009511                       1103 00105$:
                                   1104 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 585: else if ( clocksource == CLK_SOURCE_LSI)
      009511 7B 01            [ 1] 1105 	ld	a, (0x01, sp)
      009513 A1 D2            [ 1] 1106 	cp	a, #0xd2
      009515 26 0B            [ 1] 1107 	jrne	00102$
                                   1108 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 587: clockfrequency = LSI_VALUE;
      009517 AE F4 00         [ 2] 1109 	ldw	x, #0xf400
      00951A 1F 04            [ 2] 1110 	ldw	(0x04, sp), x
      00951C 90 AE 00 01      [ 2] 1111 	ldw	y, #0x0001
      009520 20 09            [ 2] 1112 	jra	00106$
      009522                       1113 00102$:
                                   1114 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 591: clockfrequency = HSE_VALUE;
      009522 AE 36 00         [ 2] 1115 	ldw	x, #0x3600
      009525 1F 04            [ 2] 1116 	ldw	(0x04, sp), x
      009527 90 AE 01 6E      [ 2] 1117 	ldw	y, #0x016e
      00952B                       1118 00106$:
                                   1119 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 594: return((uint32_t)clockfrequency);
      00952B 1E 04            [ 2] 1120 	ldw	x, (0x04, sp)
      00952D 5B 07            [ 2] 1121 	addw	sp, #7
      00952F 81               [ 4] 1122 	ret
                                   1123 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 604: void CLK_AdjustHSICalibrationValue(CLK_HSITrimValue_TypeDef CLK_HSICalibrationValue)
                                   1124 ;	-----------------------------------------
                                   1125 ;	 function CLK_AdjustHSICalibrationValue
                                   1126 ;	-----------------------------------------
      009530                       1127 _CLK_AdjustHSICalibrationValue:
                                   1128 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 607: assert_param(IS_CLK_HSITRIMVALUE_OK(CLK_HSICalibrationValue));
      009530 0D 03            [ 1] 1129 	tnz	(0x03, sp)
      009532 27 3B            [ 1] 1130 	jreq	00104$
      009534 7B 03            [ 1] 1131 	ld	a, (0x03, sp)
      009536 A1 01            [ 1] 1132 	cp	a, #0x01
      009538 27 35            [ 1] 1133 	jreq	00104$
      00953A 7B 03            [ 1] 1134 	ld	a, (0x03, sp)
      00953C A1 02            [ 1] 1135 	cp	a, #0x02
      00953E 27 2F            [ 1] 1136 	jreq	00104$
      009540 7B 03            [ 1] 1137 	ld	a, (0x03, sp)
      009542 A1 03            [ 1] 1138 	cp	a, #0x03
      009544 27 29            [ 1] 1139 	jreq	00104$
      009546 7B 03            [ 1] 1140 	ld	a, (0x03, sp)
      009548 A1 04            [ 1] 1141 	cp	a, #0x04
      00954A 27 23            [ 1] 1142 	jreq	00104$
      00954C 7B 03            [ 1] 1143 	ld	a, (0x03, sp)
      00954E A1 05            [ 1] 1144 	cp	a, #0x05
      009550 27 1D            [ 1] 1145 	jreq	00104$
      009552 7B 03            [ 1] 1146 	ld	a, (0x03, sp)
      009554 A1 06            [ 1] 1147 	cp	a, #0x06
      009556 27 17            [ 1] 1148 	jreq	00104$
      009558 7B 03            [ 1] 1149 	ld	a, (0x03, sp)
      00955A A1 07            [ 1] 1150 	cp	a, #0x07
      00955C 27 11            [ 1] 1151 	jreq	00104$
      00955E AE 96 B5         [ 2] 1152 	ldw	x, #___str_0+0
      009561 4B 5F            [ 1] 1153 	push	#0x5f
      009563 4B 02            [ 1] 1154 	push	#0x02
      009565 4B 00            [ 1] 1155 	push	#0x00
      009567 4B 00            [ 1] 1156 	push	#0x00
      009569 89               [ 2] 1157 	pushw	x
      00956A CD 80 A8         [ 4] 1158 	call	_assert_failed
      00956D 5B 06            [ 2] 1159 	addw	sp, #6
      00956F                       1160 00104$:
                                   1161 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 610: CLK->HSITRIMR = (uint8_t)( (uint8_t)(CLK->HSITRIMR & (uint8_t)(~CLK_HSITRIMR_HSITRIM))|((uint8_t)CLK_HSICalibrationValue));
      00956F AE 50 CC         [ 2] 1162 	ldw	x, #0x50cc
      009572 F6               [ 1] 1163 	ld	a, (x)
      009573 A4 F8            [ 1] 1164 	and	a, #0xf8
      009575 1A 03            [ 1] 1165 	or	a, (0x03, sp)
      009577 AE 50 CC         [ 2] 1166 	ldw	x, #0x50cc
      00957A F7               [ 1] 1167 	ld	(x), a
      00957B 81               [ 4] 1168 	ret
                                   1169 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 622: void CLK_SYSCLKEmergencyClear(void)
                                   1170 ;	-----------------------------------------
                                   1171 ;	 function CLK_SYSCLKEmergencyClear
                                   1172 ;	-----------------------------------------
      00957C                       1173 _CLK_SYSCLKEmergencyClear:
                                   1174 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 624: CLK->SWCR &= (uint8_t)(~CLK_SWCR_SWBSY);
      00957C 72 11 50 C5      [ 1] 1175 	bres	0x50c5, #0
      009580 81               [ 4] 1176 	ret
                                   1177 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 634: FlagStatus CLK_GetFlagStatus(CLK_Flag_TypeDef CLK_FLAG)
                                   1178 ;	-----------------------------------------
                                   1179 ;	 function CLK_GetFlagStatus
                                   1180 ;	-----------------------------------------
      009581                       1181 _CLK_GetFlagStatus:
      009581 88               [ 1] 1182 	push	a
                                   1183 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 641: assert_param(IS_CLK_FLAG_OK(CLK_FLAG));
      009582 1E 04            [ 2] 1184 	ldw	x, (0x04, sp)
      009584 A3 01 10         [ 2] 1185 	cpw	x, #0x0110
      009587 27 49            [ 1] 1186 	jreq	00119$
      009589 1E 04            [ 2] 1187 	ldw	x, (0x04, sp)
      00958B A3 01 02         [ 2] 1188 	cpw	x, #0x0102
      00958E 27 42            [ 1] 1189 	jreq	00119$
      009590 1E 04            [ 2] 1190 	ldw	x, (0x04, sp)
      009592 A3 02 02         [ 2] 1191 	cpw	x, #0x0202
      009595 27 3B            [ 1] 1192 	jreq	00119$
      009597 1E 04            [ 2] 1193 	ldw	x, (0x04, sp)
      009599 A3 03 08         [ 2] 1194 	cpw	x, #0x0308
      00959C 27 34            [ 1] 1195 	jreq	00119$
      00959E 1E 04            [ 2] 1196 	ldw	x, (0x04, sp)
      0095A0 A3 03 01         [ 2] 1197 	cpw	x, #0x0301
      0095A3 27 2D            [ 1] 1198 	jreq	00119$
      0095A5 1E 04            [ 2] 1199 	ldw	x, (0x04, sp)
      0095A7 A3 04 08         [ 2] 1200 	cpw	x, #0x0408
      0095AA 27 26            [ 1] 1201 	jreq	00119$
      0095AC 1E 04            [ 2] 1202 	ldw	x, (0x04, sp)
      0095AE A3 04 02         [ 2] 1203 	cpw	x, #0x0402
      0095B1 27 1F            [ 1] 1204 	jreq	00119$
      0095B3 1E 04            [ 2] 1205 	ldw	x, (0x04, sp)
      0095B5 A3 05 04         [ 2] 1206 	cpw	x, #0x0504
      0095B8 27 18            [ 1] 1207 	jreq	00119$
      0095BA 1E 04            [ 2] 1208 	ldw	x, (0x04, sp)
      0095BC A3 05 02         [ 2] 1209 	cpw	x, #0x0502
      0095BF 27 11            [ 1] 1210 	jreq	00119$
      0095C1 AE 96 B5         [ 2] 1211 	ldw	x, #___str_0+0
      0095C4 4B 81            [ 1] 1212 	push	#0x81
      0095C6 4B 02            [ 1] 1213 	push	#0x02
      0095C8 4B 00            [ 1] 1214 	push	#0x00
      0095CA 4B 00            [ 1] 1215 	push	#0x00
      0095CC 89               [ 2] 1216 	pushw	x
      0095CD CD 80 A8         [ 4] 1217 	call	_assert_failed
      0095D0 5B 06            [ 2] 1218 	addw	sp, #6
      0095D2                       1219 00119$:
                                   1220 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 644: statusreg = (uint16_t)((uint16_t)CLK_FLAG & (uint16_t)0xFF00);
      0095D2 4F               [ 1] 1221 	clr	a
      0095D3 97               [ 1] 1222 	ld	xl, a
      0095D4 7B 04            [ 1] 1223 	ld	a, (0x04, sp)
      0095D6 95               [ 1] 1224 	ld	xh, a
                                   1225 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 647: if (statusreg == 0x0100) /* The flag to check is in ICKRregister */
      0095D7 A3 01 00         [ 2] 1226 	cpw	x, #0x0100
      0095DA 26 06            [ 1] 1227 	jrne	00111$
                                   1228 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 649: tmpreg = CLK->ICKR;
      0095DC AE 50 C0         [ 2] 1229 	ldw	x, #0x50c0
      0095DF F6               [ 1] 1230 	ld	a, (x)
      0095E0 20 25            [ 2] 1231 	jra	00112$
      0095E2                       1232 00111$:
                                   1233 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 651: else if (statusreg == 0x0200) /* The flag to check is in ECKRregister */
      0095E2 A3 02 00         [ 2] 1234 	cpw	x, #0x0200
      0095E5 26 06            [ 1] 1235 	jrne	00108$
                                   1236 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 653: tmpreg = CLK->ECKR;
      0095E7 AE 50 C1         [ 2] 1237 	ldw	x, #0x50c1
      0095EA F6               [ 1] 1238 	ld	a, (x)
      0095EB 20 1A            [ 2] 1239 	jra	00112$
      0095ED                       1240 00108$:
                                   1241 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 655: else if (statusreg == 0x0300) /* The flag to check is in SWIC register */
      0095ED A3 03 00         [ 2] 1242 	cpw	x, #0x0300
      0095F0 26 06            [ 1] 1243 	jrne	00105$
                                   1244 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 657: tmpreg = CLK->SWCR;
      0095F2 AE 50 C5         [ 2] 1245 	ldw	x, #0x50c5
      0095F5 F6               [ 1] 1246 	ld	a, (x)
      0095F6 20 0F            [ 2] 1247 	jra	00112$
      0095F8                       1248 00105$:
                                   1249 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 659: else if (statusreg == 0x0400) /* The flag to check is in CSS register */
      0095F8 A3 04 00         [ 2] 1250 	cpw	x, #0x0400
      0095FB 26 06            [ 1] 1251 	jrne	00102$
                                   1252 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 661: tmpreg = CLK->CSSR;
      0095FD AE 50 C8         [ 2] 1253 	ldw	x, #0x50c8
      009600 F6               [ 1] 1254 	ld	a, (x)
      009601 20 04            [ 2] 1255 	jra	00112$
      009603                       1256 00102$:
                                   1257 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 665: tmpreg = CLK->CCOR;
      009603 AE 50 C9         [ 2] 1258 	ldw	x, #0x50c9
      009606 F6               [ 1] 1259 	ld	a, (x)
      009607                       1260 00112$:
                                   1261 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 668: if ((tmpreg & (uint8_t)CLK_FLAG) != (uint8_t)RESET)
      009607 88               [ 1] 1262 	push	a
      009608 7B 06            [ 1] 1263 	ld	a, (0x06, sp)
      00960A 6B 02            [ 1] 1264 	ld	(0x02, sp), a
      00960C 84               [ 1] 1265 	pop	a
      00960D 14 01            [ 1] 1266 	and	a, (0x01, sp)
      00960F 4D               [ 1] 1267 	tnz	a
      009610 27 03            [ 1] 1268 	jreq	00114$
                                   1269 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 670: bitstatus = SET;
      009612 A6 01            [ 1] 1270 	ld	a, #0x01
                                   1271 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 674: bitstatus = RESET;
      009614 21                    1272 	.byte 0x21
      009615                       1273 00114$:
      009615 4F               [ 1] 1274 	clr	a
      009616                       1275 00115$:
                                   1276 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 678: return((FlagStatus)bitstatus);
      009616 5B 01            [ 2] 1277 	addw	sp, #1
      009618 81               [ 4] 1278 	ret
                                   1279 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 687: ITStatus CLK_GetITStatus(CLK_IT_TypeDef CLK_IT)
                                   1280 ;	-----------------------------------------
                                   1281 ;	 function CLK_GetITStatus
                                   1282 ;	-----------------------------------------
      009619                       1283 _CLK_GetITStatus:
      009619 88               [ 1] 1284 	push	a
                                   1285 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 692: assert_param(IS_CLK_IT_OK(CLK_IT));
      00961A 7B 04            [ 1] 1286 	ld	a, (0x04, sp)
      00961C A1 1C            [ 1] 1287 	cp	a, #0x1c
      00961E 26 06            [ 1] 1288 	jrne	00138$
      009620 A6 01            [ 1] 1289 	ld	a, #0x01
      009622 6B 01            [ 1] 1290 	ld	(0x01, sp), a
      009624 20 02            [ 2] 1291 	jra	00139$
      009626                       1292 00138$:
      009626 0F 01            [ 1] 1293 	clr	(0x01, sp)
      009628                       1294 00139$:
      009628 7B 04            [ 1] 1295 	ld	a, (0x04, sp)
      00962A A1 0C            [ 1] 1296 	cp	a, #0x0c
      00962C 27 15            [ 1] 1297 	jreq	00113$
      00962E 0D 01            [ 1] 1298 	tnz	(0x01, sp)
      009630 26 11            [ 1] 1299 	jrne	00113$
      009632 AE 96 B5         [ 2] 1300 	ldw	x, #___str_0+0
      009635 4B B4            [ 1] 1301 	push	#0xb4
      009637 4B 02            [ 1] 1302 	push	#0x02
      009639 4B 00            [ 1] 1303 	push	#0x00
      00963B 4B 00            [ 1] 1304 	push	#0x00
      00963D 89               [ 2] 1305 	pushw	x
      00963E CD 80 A8         [ 4] 1306 	call	_assert_failed
      009641 5B 06            [ 2] 1307 	addw	sp, #6
      009643                       1308 00113$:
                                   1309 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 694: if (CLK_IT == CLK_IT_SWIF)
      009643 0D 01            [ 1] 1310 	tnz	(0x01, sp)
      009645 27 11            [ 1] 1311 	jreq	00108$
                                   1312 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 697: if ((CLK->SWCR & (uint8_t)CLK_IT) == (uint8_t)0x0C)
      009647 AE 50 C5         [ 2] 1313 	ldw	x, #0x50c5
      00964A F6               [ 1] 1314 	ld	a, (x)
      00964B 14 04            [ 1] 1315 	and	a, (0x04, sp)
      00964D A1 0C            [ 1] 1316 	cp	a, #0x0c
      00964F 26 04            [ 1] 1317 	jrne	00102$
                                   1318 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 699: bitstatus = SET;
      009651 A6 01            [ 1] 1319 	ld	a, #0x01
      009653 20 11            [ 2] 1320 	jra	00109$
      009655                       1321 00102$:
                                   1322 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 703: bitstatus = RESET;
      009655 4F               [ 1] 1323 	clr	a
      009656 20 0E            [ 2] 1324 	jra	00109$
      009658                       1325 00108$:
                                   1326 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 709: if ((CLK->CSSR & (uint8_t)CLK_IT) == (uint8_t)0x0C)
      009658 AE 50 C8         [ 2] 1327 	ldw	x, #0x50c8
      00965B F6               [ 1] 1328 	ld	a, (x)
      00965C 14 04            [ 1] 1329 	and	a, (0x04, sp)
      00965E A1 0C            [ 1] 1330 	cp	a, #0x0c
      009660 26 03            [ 1] 1331 	jrne	00105$
                                   1332 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 711: bitstatus = SET;
      009662 A6 01            [ 1] 1333 	ld	a, #0x01
                                   1334 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 715: bitstatus = RESET;
      009664 21                    1335 	.byte 0x21
      009665                       1336 00105$:
      009665 4F               [ 1] 1337 	clr	a
      009666                       1338 00109$:
                                   1339 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 720: return bitstatus;
      009666 5B 01            [ 2] 1340 	addw	sp, #1
      009668 81               [ 4] 1341 	ret
                                   1342 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 729: void CLK_ClearITPendingBit(CLK_IT_TypeDef CLK_IT)
                                   1343 ;	-----------------------------------------
                                   1344 ;	 function CLK_ClearITPendingBit
                                   1345 ;	-----------------------------------------
      009669                       1346 _CLK_ClearITPendingBit:
      009669 88               [ 1] 1347 	push	a
                                   1348 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 732: assert_param(IS_CLK_IT_OK(CLK_IT));
      00966A 7B 04            [ 1] 1349 	ld	a, (0x04, sp)
      00966C A1 0C            [ 1] 1350 	cp	a, #0x0c
      00966E 26 06            [ 1] 1351 	jrne	00124$
      009670 A6 01            [ 1] 1352 	ld	a, #0x01
      009672 6B 01            [ 1] 1353 	ld	(0x01, sp), a
      009674 20 02            [ 2] 1354 	jra	00125$
      009676                       1355 00124$:
      009676 0F 01            [ 1] 1356 	clr	(0x01, sp)
      009678                       1357 00125$:
      009678 0D 01            [ 1] 1358 	tnz	(0x01, sp)
      00967A 26 17            [ 1] 1359 	jrne	00107$
      00967C 7B 04            [ 1] 1360 	ld	a, (0x04, sp)
      00967E A1 1C            [ 1] 1361 	cp	a, #0x1c
      009680 27 11            [ 1] 1362 	jreq	00107$
      009682 AE 96 B5         [ 2] 1363 	ldw	x, #___str_0+0
      009685 4B DC            [ 1] 1364 	push	#0xdc
      009687 4B 02            [ 1] 1365 	push	#0x02
      009689 4B 00            [ 1] 1366 	push	#0x00
      00968B 4B 00            [ 1] 1367 	push	#0x00
      00968D 89               [ 2] 1368 	pushw	x
      00968E CD 80 A8         [ 4] 1369 	call	_assert_failed
      009691 5B 06            [ 2] 1370 	addw	sp, #6
      009693                       1371 00107$:
                                   1372 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 734: if (CLK_IT == (uint8_t)CLK_IT_CSSD)
      009693 0D 01            [ 1] 1373 	tnz	(0x01, sp)
      009695 27 09            [ 1] 1374 	jreq	00102$
                                   1375 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 737: CLK->CSSR &= (uint8_t)(~CLK_CSSR_CSSD);
      009697 AE 50 C8         [ 2] 1376 	ldw	x, #0x50c8
      00969A F6               [ 1] 1377 	ld	a, (x)
      00969B A4 F7            [ 1] 1378 	and	a, #0xf7
      00969D F7               [ 1] 1379 	ld	(x), a
      00969E 20 07            [ 2] 1380 	jra	00104$
      0096A0                       1381 00102$:
                                   1382 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c: 742: CLK->SWCR &= (uint8_t)(~CLK_SWCR_SWIF);
      0096A0 AE 50 C5         [ 2] 1383 	ldw	x, #0x50c5
      0096A3 F6               [ 1] 1384 	ld	a, (x)
      0096A4 A4 F7            [ 1] 1385 	and	a, #0xf7
      0096A6 F7               [ 1] 1386 	ld	(x), a
      0096A7                       1387 00104$:
      0096A7 84               [ 1] 1388 	pop	a
      0096A8 81               [ 4] 1389 	ret
                                   1390 	.area CODE
      0096A9                       1391 _HSIDivFactor:
      0096A9 01                    1392 	.db #0x01	; 1
      0096AA 02                    1393 	.db #0x02	; 2
      0096AB 04                    1394 	.db #0x04	; 4
      0096AC 08                    1395 	.db #0x08	; 8
      0096AD                       1396 _CLKPrescTable:
      0096AD 01                    1397 	.db #0x01	; 1
      0096AE 02                    1398 	.db #0x02	; 2
      0096AF 04                    1399 	.db #0x04	; 4
      0096B0 08                    1400 	.db #0x08	; 8
      0096B1 0A                    1401 	.db #0x0a	; 10
      0096B2 10                    1402 	.db #0x10	; 16
      0096B3 14                    1403 	.db #0x14	; 20
      0096B4 28                    1404 	.db #0x28	; 40
      0096B5                       1405 ___str_0:
      0096B5 2E 2E 2F 2E 2E 2F 6D  1406 	.ascii "../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_clk.c"
             69 70 6F 73 2F 61 72
             63 68 2F 73 74 6D 38
             2F 73 74 6D 38 73 2D
             73 64 63 63 2F 73 72
             63 2F 73 74 6D 38 73
             5F 63 6C 6B 2E 63
      0096E5 00                    1407 	.db 0x00
                                   1408 	.area INITIALIZER
                                   1409 	.area CABS (ABS)
