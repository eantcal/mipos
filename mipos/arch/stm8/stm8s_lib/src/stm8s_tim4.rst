                                      1 ;--------------------------------------------------------
                                      2 ; File Created by SDCC : free open source ANSI-C Compiler
                                      3 ; Version 3.6.0 #9615 (Linux)
                                      4 ;--------------------------------------------------------
                                      5 	.module stm8s_tim4
                                      6 	.optsdcc -mstm8
                                      7 	
                                      8 ;--------------------------------------------------------
                                      9 ; Public variables in this module
                                     10 ;--------------------------------------------------------
                                     11 	.globl _assert_failed
                                     12 	.globl _TIM4_DeInit
                                     13 	.globl _TIM4_TimeBaseInit
                                     14 	.globl _TIM4_Cmd
                                     15 	.globl _TIM4_ITConfig
                                     16 	.globl _TIM4_UpdateDisableConfig
                                     17 	.globl _TIM4_UpdateRequestConfig
                                     18 	.globl _TIM4_SelectOnePulseMode
                                     19 	.globl _TIM4_PrescalerConfig
                                     20 	.globl _TIM4_ARRPreloadConfig
                                     21 	.globl _TIM4_GenerateEvent
                                     22 	.globl _TIM4_SetCounter
                                     23 	.globl _TIM4_SetAutoreload
                                     24 	.globl _TIM4_GetCounter
                                     25 	.globl _TIM4_GetPrescaler
                                     26 	.globl _TIM4_GetFlagStatus
                                     27 	.globl _TIM4_ClearFlag
                                     28 	.globl _TIM4_GetITStatus
                                     29 	.globl _TIM4_ClearITPendingBit
                                     30 ;--------------------------------------------------------
                                     31 ; ram data
                                     32 ;--------------------------------------------------------
                                     33 	.area DATA
                                     34 ;--------------------------------------------------------
                                     35 ; ram data
                                     36 ;--------------------------------------------------------
                                     37 	.area INITIALIZED
                                     38 ;--------------------------------------------------------
                                     39 ; absolute external ram data
                                     40 ;--------------------------------------------------------
                                     41 	.area DABS (ABS)
                                     42 ;--------------------------------------------------------
                                     43 ; global & static initialisations
                                     44 ;--------------------------------------------------------
                                     45 	.area HOME
                                     46 	.area GSINIT
                                     47 	.area GSFINAL
                                     48 	.area GSINIT
                                     49 ;--------------------------------------------------------
                                     50 ; Home
                                     51 ;--------------------------------------------------------
                                     52 	.area HOME
                                     53 	.area HOME
                                     54 ;--------------------------------------------------------
                                     55 ; code
                                     56 ;--------------------------------------------------------
                                     57 	.area CODE
                                     58 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 49: void TIM4_DeInit(void)
                                     59 ;	-----------------------------------------
                                     60 ;	 function TIM4_DeInit
                                     61 ;	-----------------------------------------
      008C3E                         62 _TIM4_DeInit:
                                     63 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 51: TIM4->CR1 = TIM4_CR1_RESET_VALUE;
      008C3E 35 00 53 40      [ 1]   64 	mov	0x5340+0, #0x00
                                     65 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 52: TIM4->IER = TIM4_IER_RESET_VALUE;
      008C42 35 00 53 41      [ 1]   66 	mov	0x5341+0, #0x00
                                     67 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 53: TIM4->CNTR = TIM4_CNTR_RESET_VALUE;
      008C46 35 00 53 44      [ 1]   68 	mov	0x5344+0, #0x00
                                     69 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 54: TIM4->PSCR = TIM4_PSCR_RESET_VALUE;
      008C4A 35 00 53 45      [ 1]   70 	mov	0x5345+0, #0x00
                                     71 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 55: TIM4->ARR = TIM4_ARR_RESET_VALUE;
      008C4E 35 FF 53 46      [ 1]   72 	mov	0x5346+0, #0xff
                                     73 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 56: TIM4->SR1 = TIM4_SR1_RESET_VALUE;
      008C52 35 00 53 42      [ 1]   74 	mov	0x5342+0, #0x00
      008C56 81               [ 4]   75 	ret
                                     76 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 65: void TIM4_TimeBaseInit(TIM4_Prescaler_TypeDef TIM4_Prescaler, uint8_t TIM4_Period)
                                     77 ;	-----------------------------------------
                                     78 ;	 function TIM4_TimeBaseInit
                                     79 ;	-----------------------------------------
      008C57                         80 _TIM4_TimeBaseInit:
                                     81 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 68: assert_param(IS_TIM4_PRESCALER_OK(TIM4_Prescaler));
      008C57 0D 03            [ 1]   82 	tnz	(0x03, sp)
      008C59 27 3B            [ 1]   83 	jreq	00104$
      008C5B 7B 03            [ 1]   84 	ld	a, (0x03, sp)
      008C5D A1 01            [ 1]   85 	cp	a, #0x01
      008C5F 27 35            [ 1]   86 	jreq	00104$
      008C61 7B 03            [ 1]   87 	ld	a, (0x03, sp)
      008C63 A1 02            [ 1]   88 	cp	a, #0x02
      008C65 27 2F            [ 1]   89 	jreq	00104$
      008C67 7B 03            [ 1]   90 	ld	a, (0x03, sp)
      008C69 A1 03            [ 1]   91 	cp	a, #0x03
      008C6B 27 29            [ 1]   92 	jreq	00104$
      008C6D 7B 03            [ 1]   93 	ld	a, (0x03, sp)
      008C6F A1 04            [ 1]   94 	cp	a, #0x04
      008C71 27 23            [ 1]   95 	jreq	00104$
      008C73 7B 03            [ 1]   96 	ld	a, (0x03, sp)
      008C75 A1 05            [ 1]   97 	cp	a, #0x05
      008C77 27 1D            [ 1]   98 	jreq	00104$
      008C79 7B 03            [ 1]   99 	ld	a, (0x03, sp)
      008C7B A1 06            [ 1]  100 	cp	a, #0x06
      008C7D 27 17            [ 1]  101 	jreq	00104$
      008C7F 7B 03            [ 1]  102 	ld	a, (0x03, sp)
      008C81 A1 07            [ 1]  103 	cp	a, #0x07
      008C83 27 11            [ 1]  104 	jreq	00104$
      008C85 AE 8F 0A         [ 2]  105 	ldw	x, #___str_0+0
      008C88 4B 44            [ 1]  106 	push	#0x44
      008C8A 4B 00            [ 1]  107 	push	#0x00
      008C8C 4B 00            [ 1]  108 	push	#0x00
      008C8E 4B 00            [ 1]  109 	push	#0x00
      008C90 89               [ 2]  110 	pushw	x
      008C91 CD 80 A8         [ 4]  111 	call	_assert_failed
      008C94 5B 06            [ 2]  112 	addw	sp, #6
      008C96                        113 00104$:
                                    114 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 70: TIM4->PSCR = (uint8_t)(TIM4_Prescaler);
      008C96 AE 53 45         [ 2]  115 	ldw	x, #0x5345
      008C99 7B 03            [ 1]  116 	ld	a, (0x03, sp)
      008C9B F7               [ 1]  117 	ld	(x), a
                                    118 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 72: TIM4->ARR = (uint8_t)(TIM4_Period);
      008C9C AE 53 46         [ 2]  119 	ldw	x, #0x5346
      008C9F 7B 04            [ 1]  120 	ld	a, (0x04, sp)
      008CA1 F7               [ 1]  121 	ld	(x), a
      008CA2 81               [ 4]  122 	ret
                                    123 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 81: void TIM4_Cmd(FunctionalState NewState)
                                    124 ;	-----------------------------------------
                                    125 ;	 function TIM4_Cmd
                                    126 ;	-----------------------------------------
      008CA3                        127 _TIM4_Cmd:
                                    128 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 84: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008CA3 0D 03            [ 1]  129 	tnz	(0x03, sp)
      008CA5 27 17            [ 1]  130 	jreq	00107$
      008CA7 7B 03            [ 1]  131 	ld	a, (0x03, sp)
      008CA9 A1 01            [ 1]  132 	cp	a, #0x01
      008CAB 27 11            [ 1]  133 	jreq	00107$
      008CAD AE 8F 0A         [ 2]  134 	ldw	x, #___str_0+0
      008CB0 4B 54            [ 1]  135 	push	#0x54
      008CB2 4B 00            [ 1]  136 	push	#0x00
      008CB4 4B 00            [ 1]  137 	push	#0x00
      008CB6 4B 00            [ 1]  138 	push	#0x00
      008CB8 89               [ 2]  139 	pushw	x
      008CB9 CD 80 A8         [ 4]  140 	call	_assert_failed
      008CBC 5B 06            [ 2]  141 	addw	sp, #6
      008CBE                        142 00107$:
                                    143 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 87: if (NewState != DISABLE)
      008CBE 0D 03            [ 1]  144 	tnz	(0x03, sp)
      008CC0 27 05            [ 1]  145 	jreq	00102$
                                    146 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 89: TIM4->CR1 |= TIM4_CR1_CEN;
      008CC2 72 10 53 40      [ 1]  147 	bset	0x5340, #0
      008CC6 81               [ 4]  148 	ret
      008CC7                        149 00102$:
                                    150 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 93: TIM4->CR1 &= (uint8_t)(~TIM4_CR1_CEN);
      008CC7 72 11 53 40      [ 1]  151 	bres	0x5340, #0
      008CCB 81               [ 4]  152 	ret
                                    153 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 107: void TIM4_ITConfig(TIM4_IT_TypeDef TIM4_IT, FunctionalState NewState)
                                    154 ;	-----------------------------------------
                                    155 ;	 function TIM4_ITConfig
                                    156 ;	-----------------------------------------
      008CCC                        157 _TIM4_ITConfig:
      008CCC 88               [ 1]  158 	push	a
                                    159 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 110: assert_param(IS_TIM4_IT_OK(TIM4_IT));
      008CCD 7B 04            [ 1]  160 	ld	a, (0x04, sp)
      008CCF A1 01            [ 1]  161 	cp	a, #0x01
      008CD1 27 11            [ 1]  162 	jreq	00107$
      008CD3 AE 8F 0A         [ 2]  163 	ldw	x, #___str_0+0
      008CD6 4B 6E            [ 1]  164 	push	#0x6e
      008CD8 4B 00            [ 1]  165 	push	#0x00
      008CDA 4B 00            [ 1]  166 	push	#0x00
      008CDC 4B 00            [ 1]  167 	push	#0x00
      008CDE 89               [ 2]  168 	pushw	x
      008CDF CD 80 A8         [ 4]  169 	call	_assert_failed
      008CE2 5B 06            [ 2]  170 	addw	sp, #6
      008CE4                        171 00107$:
                                    172 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 111: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008CE4 0D 05            [ 1]  173 	tnz	(0x05, sp)
      008CE6 27 17            [ 1]  174 	jreq	00109$
      008CE8 7B 05            [ 1]  175 	ld	a, (0x05, sp)
      008CEA A1 01            [ 1]  176 	cp	a, #0x01
      008CEC 27 11            [ 1]  177 	jreq	00109$
      008CEE AE 8F 0A         [ 2]  178 	ldw	x, #___str_0+0
      008CF1 4B 6F            [ 1]  179 	push	#0x6f
      008CF3 4B 00            [ 1]  180 	push	#0x00
      008CF5 4B 00            [ 1]  181 	push	#0x00
      008CF7 4B 00            [ 1]  182 	push	#0x00
      008CF9 89               [ 2]  183 	pushw	x
      008CFA CD 80 A8         [ 4]  184 	call	_assert_failed
      008CFD 5B 06            [ 2]  185 	addw	sp, #6
      008CFF                        186 00109$:
                                    187 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 113: if (NewState != DISABLE)
      008CFF 0D 05            [ 1]  188 	tnz	(0x05, sp)
      008D01 27 0C            [ 1]  189 	jreq	00102$
                                    190 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 116: TIM4->IER |= (uint8_t)TIM4_IT;
      008D03 AE 53 41         [ 2]  191 	ldw	x, #0x5341
      008D06 F6               [ 1]  192 	ld	a, (x)
      008D07 1A 04            [ 1]  193 	or	a, (0x04, sp)
      008D09 AE 53 41         [ 2]  194 	ldw	x, #0x5341
      008D0C F7               [ 1]  195 	ld	(x), a
      008D0D 20 0F            [ 2]  196 	jra	00104$
      008D0F                        197 00102$:
                                    198 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 121: TIM4->IER &= (uint8_t)(~TIM4_IT);
      008D0F AE 53 41         [ 2]  199 	ldw	x, #0x5341
      008D12 F6               [ 1]  200 	ld	a, (x)
      008D13 6B 01            [ 1]  201 	ld	(0x01, sp), a
      008D15 7B 04            [ 1]  202 	ld	a, (0x04, sp)
      008D17 43               [ 1]  203 	cpl	a
      008D18 14 01            [ 1]  204 	and	a, (0x01, sp)
      008D1A AE 53 41         [ 2]  205 	ldw	x, #0x5341
      008D1D F7               [ 1]  206 	ld	(x), a
      008D1E                        207 00104$:
      008D1E 84               [ 1]  208 	pop	a
      008D1F 81               [ 4]  209 	ret
                                    210 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 131: void TIM4_UpdateDisableConfig(FunctionalState NewState)
                                    211 ;	-----------------------------------------
                                    212 ;	 function TIM4_UpdateDisableConfig
                                    213 ;	-----------------------------------------
      008D20                        214 _TIM4_UpdateDisableConfig:
                                    215 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 134: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008D20 0D 03            [ 1]  216 	tnz	(0x03, sp)
      008D22 27 17            [ 1]  217 	jreq	00107$
      008D24 7B 03            [ 1]  218 	ld	a, (0x03, sp)
      008D26 A1 01            [ 1]  219 	cp	a, #0x01
      008D28 27 11            [ 1]  220 	jreq	00107$
      008D2A AE 8F 0A         [ 2]  221 	ldw	x, #___str_0+0
      008D2D 4B 86            [ 1]  222 	push	#0x86
      008D2F 4B 00            [ 1]  223 	push	#0x00
      008D31 4B 00            [ 1]  224 	push	#0x00
      008D33 4B 00            [ 1]  225 	push	#0x00
      008D35 89               [ 2]  226 	pushw	x
      008D36 CD 80 A8         [ 4]  227 	call	_assert_failed
      008D39 5B 06            [ 2]  228 	addw	sp, #6
      008D3B                        229 00107$:
                                    230 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 137: if (NewState != DISABLE)
      008D3B 0D 03            [ 1]  231 	tnz	(0x03, sp)
      008D3D 27 08            [ 1]  232 	jreq	00102$
                                    233 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 139: TIM4->CR1 |= TIM4_CR1_UDIS;
      008D3F AE 53 40         [ 2]  234 	ldw	x, #0x5340
      008D42 F6               [ 1]  235 	ld	a, (x)
      008D43 AA 02            [ 1]  236 	or	a, #0x02
      008D45 F7               [ 1]  237 	ld	(x), a
      008D46 81               [ 4]  238 	ret
      008D47                        239 00102$:
                                    240 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 143: TIM4->CR1 &= (uint8_t)(~TIM4_CR1_UDIS);
      008D47 AE 53 40         [ 2]  241 	ldw	x, #0x5340
      008D4A F6               [ 1]  242 	ld	a, (x)
      008D4B A4 FD            [ 1]  243 	and	a, #0xfd
      008D4D F7               [ 1]  244 	ld	(x), a
      008D4E 81               [ 4]  245 	ret
                                    246 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 155: void TIM4_UpdateRequestConfig(TIM4_UpdateSource_TypeDef TIM4_UpdateSource)
                                    247 ;	-----------------------------------------
                                    248 ;	 function TIM4_UpdateRequestConfig
                                    249 ;	-----------------------------------------
      008D4F                        250 _TIM4_UpdateRequestConfig:
                                    251 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 158: assert_param(IS_TIM4_UPDATE_SOURCE_OK(TIM4_UpdateSource));
      008D4F 0D 03            [ 1]  252 	tnz	(0x03, sp)
      008D51 27 17            [ 1]  253 	jreq	00107$
      008D53 7B 03            [ 1]  254 	ld	a, (0x03, sp)
      008D55 A1 01            [ 1]  255 	cp	a, #0x01
      008D57 27 11            [ 1]  256 	jreq	00107$
      008D59 AE 8F 0A         [ 2]  257 	ldw	x, #___str_0+0
      008D5C 4B 9E            [ 1]  258 	push	#0x9e
      008D5E 4B 00            [ 1]  259 	push	#0x00
      008D60 4B 00            [ 1]  260 	push	#0x00
      008D62 4B 00            [ 1]  261 	push	#0x00
      008D64 89               [ 2]  262 	pushw	x
      008D65 CD 80 A8         [ 4]  263 	call	_assert_failed
      008D68 5B 06            [ 2]  264 	addw	sp, #6
      008D6A                        265 00107$:
                                    266 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 161: if (TIM4_UpdateSource != TIM4_UPDATESOURCE_GLOBAL)
      008D6A 0D 03            [ 1]  267 	tnz	(0x03, sp)
      008D6C 27 08            [ 1]  268 	jreq	00102$
                                    269 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 163: TIM4->CR1 |= TIM4_CR1_URS;
      008D6E AE 53 40         [ 2]  270 	ldw	x, #0x5340
      008D71 F6               [ 1]  271 	ld	a, (x)
      008D72 AA 04            [ 1]  272 	or	a, #0x04
      008D74 F7               [ 1]  273 	ld	(x), a
      008D75 81               [ 4]  274 	ret
      008D76                        275 00102$:
                                    276 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 167: TIM4->CR1 &= (uint8_t)(~TIM4_CR1_URS);
      008D76 AE 53 40         [ 2]  277 	ldw	x, #0x5340
      008D79 F6               [ 1]  278 	ld	a, (x)
      008D7A A4 FB            [ 1]  279 	and	a, #0xfb
      008D7C F7               [ 1]  280 	ld	(x), a
      008D7D 81               [ 4]  281 	ret
                                    282 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 179: void TIM4_SelectOnePulseMode(TIM4_OPMode_TypeDef TIM4_OPMode)
                                    283 ;	-----------------------------------------
                                    284 ;	 function TIM4_SelectOnePulseMode
                                    285 ;	-----------------------------------------
      008D7E                        286 _TIM4_SelectOnePulseMode:
                                    287 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 182: assert_param(IS_TIM4_OPM_MODE_OK(TIM4_OPMode));
      008D7E 7B 03            [ 1]  288 	ld	a, (0x03, sp)
      008D80 A1 01            [ 1]  289 	cp	a, #0x01
      008D82 27 15            [ 1]  290 	jreq	00107$
      008D84 0D 03            [ 1]  291 	tnz	(0x03, sp)
      008D86 27 11            [ 1]  292 	jreq	00107$
      008D88 AE 8F 0A         [ 2]  293 	ldw	x, #___str_0+0
      008D8B 4B B6            [ 1]  294 	push	#0xb6
      008D8D 4B 00            [ 1]  295 	push	#0x00
      008D8F 4B 00            [ 1]  296 	push	#0x00
      008D91 4B 00            [ 1]  297 	push	#0x00
      008D93 89               [ 2]  298 	pushw	x
      008D94 CD 80 A8         [ 4]  299 	call	_assert_failed
      008D97 5B 06            [ 2]  300 	addw	sp, #6
      008D99                        301 00107$:
                                    302 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 185: if (TIM4_OPMode != TIM4_OPMODE_REPETITIVE)
      008D99 0D 03            [ 1]  303 	tnz	(0x03, sp)
      008D9B 27 08            [ 1]  304 	jreq	00102$
                                    305 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 187: TIM4->CR1 |= TIM4_CR1_OPM;
      008D9D AE 53 40         [ 2]  306 	ldw	x, #0x5340
      008DA0 F6               [ 1]  307 	ld	a, (x)
      008DA1 AA 08            [ 1]  308 	or	a, #0x08
      008DA3 F7               [ 1]  309 	ld	(x), a
      008DA4 81               [ 4]  310 	ret
      008DA5                        311 00102$:
                                    312 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 191: TIM4->CR1 &= (uint8_t)(~TIM4_CR1_OPM);
      008DA5 AE 53 40         [ 2]  313 	ldw	x, #0x5340
      008DA8 F6               [ 1]  314 	ld	a, (x)
      008DA9 A4 F7            [ 1]  315 	and	a, #0xf7
      008DAB F7               [ 1]  316 	ld	(x), a
      008DAC 81               [ 4]  317 	ret
                                    318 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 215: void TIM4_PrescalerConfig(TIM4_Prescaler_TypeDef Prescaler, TIM4_PSCReloadMode_TypeDef TIM4_PSCReloadMode)
                                    319 ;	-----------------------------------------
                                    320 ;	 function TIM4_PrescalerConfig
                                    321 ;	-----------------------------------------
      008DAD                        322 _TIM4_PrescalerConfig:
                                    323 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 218: assert_param(IS_TIM4_PRESCALER_RELOAD_OK(TIM4_PSCReloadMode));
      008DAD 0D 04            [ 1]  324 	tnz	(0x04, sp)
      008DAF 27 17            [ 1]  325 	jreq	00104$
      008DB1 7B 04            [ 1]  326 	ld	a, (0x04, sp)
      008DB3 A1 01            [ 1]  327 	cp	a, #0x01
      008DB5 27 11            [ 1]  328 	jreq	00104$
      008DB7 AE 8F 0A         [ 2]  329 	ldw	x, #___str_0+0
      008DBA 4B DA            [ 1]  330 	push	#0xda
      008DBC 4B 00            [ 1]  331 	push	#0x00
      008DBE 4B 00            [ 1]  332 	push	#0x00
      008DC0 4B 00            [ 1]  333 	push	#0x00
      008DC2 89               [ 2]  334 	pushw	x
      008DC3 CD 80 A8         [ 4]  335 	call	_assert_failed
      008DC6 5B 06            [ 2]  336 	addw	sp, #6
      008DC8                        337 00104$:
                                    338 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 219: assert_param(IS_TIM4_PRESCALER_OK(Prescaler));
      008DC8 0D 03            [ 1]  339 	tnz	(0x03, sp)
      008DCA 27 3B            [ 1]  340 	jreq	00109$
      008DCC 7B 03            [ 1]  341 	ld	a, (0x03, sp)
      008DCE A1 01            [ 1]  342 	cp	a, #0x01
      008DD0 27 35            [ 1]  343 	jreq	00109$
      008DD2 7B 03            [ 1]  344 	ld	a, (0x03, sp)
      008DD4 A1 02            [ 1]  345 	cp	a, #0x02
      008DD6 27 2F            [ 1]  346 	jreq	00109$
      008DD8 7B 03            [ 1]  347 	ld	a, (0x03, sp)
      008DDA A1 03            [ 1]  348 	cp	a, #0x03
      008DDC 27 29            [ 1]  349 	jreq	00109$
      008DDE 7B 03            [ 1]  350 	ld	a, (0x03, sp)
      008DE0 A1 04            [ 1]  351 	cp	a, #0x04
      008DE2 27 23            [ 1]  352 	jreq	00109$
      008DE4 7B 03            [ 1]  353 	ld	a, (0x03, sp)
      008DE6 A1 05            [ 1]  354 	cp	a, #0x05
      008DE8 27 1D            [ 1]  355 	jreq	00109$
      008DEA 7B 03            [ 1]  356 	ld	a, (0x03, sp)
      008DEC A1 06            [ 1]  357 	cp	a, #0x06
      008DEE 27 17            [ 1]  358 	jreq	00109$
      008DF0 7B 03            [ 1]  359 	ld	a, (0x03, sp)
      008DF2 A1 07            [ 1]  360 	cp	a, #0x07
      008DF4 27 11            [ 1]  361 	jreq	00109$
      008DF6 AE 8F 0A         [ 2]  362 	ldw	x, #___str_0+0
      008DF9 4B DB            [ 1]  363 	push	#0xdb
      008DFB 4B 00            [ 1]  364 	push	#0x00
      008DFD 4B 00            [ 1]  365 	push	#0x00
      008DFF 4B 00            [ 1]  366 	push	#0x00
      008E01 89               [ 2]  367 	pushw	x
      008E02 CD 80 A8         [ 4]  368 	call	_assert_failed
      008E05 5B 06            [ 2]  369 	addw	sp, #6
      008E07                        370 00109$:
                                    371 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 222: TIM4->PSCR = (uint8_t)Prescaler;
      008E07 AE 53 45         [ 2]  372 	ldw	x, #0x5345
      008E0A 7B 03            [ 1]  373 	ld	a, (0x03, sp)
      008E0C F7               [ 1]  374 	ld	(x), a
                                    375 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 225: TIM4->EGR = (uint8_t)TIM4_PSCReloadMode;
      008E0D AE 53 43         [ 2]  376 	ldw	x, #0x5343
      008E10 7B 04            [ 1]  377 	ld	a, (0x04, sp)
      008E12 F7               [ 1]  378 	ld	(x), a
      008E13 81               [ 4]  379 	ret
                                    380 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 234: void TIM4_ARRPreloadConfig(FunctionalState NewState)
                                    381 ;	-----------------------------------------
                                    382 ;	 function TIM4_ARRPreloadConfig
                                    383 ;	-----------------------------------------
      008E14                        384 _TIM4_ARRPreloadConfig:
                                    385 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 237: assert_param(IS_FUNCTIONALSTATE_OK(NewState));
      008E14 0D 03            [ 1]  386 	tnz	(0x03, sp)
      008E16 27 17            [ 1]  387 	jreq	00107$
      008E18 7B 03            [ 1]  388 	ld	a, (0x03, sp)
      008E1A A1 01            [ 1]  389 	cp	a, #0x01
      008E1C 27 11            [ 1]  390 	jreq	00107$
      008E1E AE 8F 0A         [ 2]  391 	ldw	x, #___str_0+0
      008E21 4B ED            [ 1]  392 	push	#0xed
      008E23 4B 00            [ 1]  393 	push	#0x00
      008E25 4B 00            [ 1]  394 	push	#0x00
      008E27 4B 00            [ 1]  395 	push	#0x00
      008E29 89               [ 2]  396 	pushw	x
      008E2A CD 80 A8         [ 4]  397 	call	_assert_failed
      008E2D 5B 06            [ 2]  398 	addw	sp, #6
      008E2F                        399 00107$:
                                    400 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 240: if (NewState != DISABLE)
      008E2F 0D 03            [ 1]  401 	tnz	(0x03, sp)
      008E31 27 05            [ 1]  402 	jreq	00102$
                                    403 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 242: TIM4->CR1 |= TIM4_CR1_ARPE;
      008E33 72 1E 53 40      [ 1]  404 	bset	0x5340, #7
      008E37 81               [ 4]  405 	ret
      008E38                        406 00102$:
                                    407 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 246: TIM4->CR1 &= (uint8_t)(~TIM4_CR1_ARPE);
      008E38 72 1F 53 40      [ 1]  408 	bres	0x5340, #7
      008E3C 81               [ 4]  409 	ret
                                    410 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 257: void TIM4_GenerateEvent(TIM4_EventSource_TypeDef TIM4_EventSource)
                                    411 ;	-----------------------------------------
                                    412 ;	 function TIM4_GenerateEvent
                                    413 ;	-----------------------------------------
      008E3D                        414 _TIM4_GenerateEvent:
                                    415 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 260: assert_param(IS_TIM4_EVENT_SOURCE_OK(TIM4_EventSource));
      008E3D 7B 03            [ 1]  416 	ld	a, (0x03, sp)
      008E3F A1 01            [ 1]  417 	cp	a, #0x01
      008E41 27 11            [ 1]  418 	jreq	00104$
      008E43 AE 8F 0A         [ 2]  419 	ldw	x, #___str_0+0
      008E46 4B 04            [ 1]  420 	push	#0x04
      008E48 4B 01            [ 1]  421 	push	#0x01
      008E4A 4B 00            [ 1]  422 	push	#0x00
      008E4C 4B 00            [ 1]  423 	push	#0x00
      008E4E 89               [ 2]  424 	pushw	x
      008E4F CD 80 A8         [ 4]  425 	call	_assert_failed
      008E52 5B 06            [ 2]  426 	addw	sp, #6
      008E54                        427 00104$:
                                    428 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 263: TIM4->EGR = (uint8_t)(TIM4_EventSource);
      008E54 AE 53 43         [ 2]  429 	ldw	x, #0x5343
      008E57 7B 03            [ 1]  430 	ld	a, (0x03, sp)
      008E59 F7               [ 1]  431 	ld	(x), a
      008E5A 81               [ 4]  432 	ret
                                    433 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 272: void TIM4_SetCounter(uint8_t Counter)
                                    434 ;	-----------------------------------------
                                    435 ;	 function TIM4_SetCounter
                                    436 ;	-----------------------------------------
      008E5B                        437 _TIM4_SetCounter:
                                    438 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 275: TIM4->CNTR = (uint8_t)(Counter);
      008E5B AE 53 44         [ 2]  439 	ldw	x, #0x5344
      008E5E 7B 03            [ 1]  440 	ld	a, (0x03, sp)
      008E60 F7               [ 1]  441 	ld	(x), a
      008E61 81               [ 4]  442 	ret
                                    443 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 284: void TIM4_SetAutoreload(uint8_t Autoreload)
                                    444 ;	-----------------------------------------
                                    445 ;	 function TIM4_SetAutoreload
                                    446 ;	-----------------------------------------
      008E62                        447 _TIM4_SetAutoreload:
                                    448 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 287: TIM4->ARR = (uint8_t)(Autoreload);
      008E62 AE 53 46         [ 2]  449 	ldw	x, #0x5346
      008E65 7B 03            [ 1]  450 	ld	a, (0x03, sp)
      008E67 F7               [ 1]  451 	ld	(x), a
      008E68 81               [ 4]  452 	ret
                                    453 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 295: uint8_t TIM4_GetCounter(void)
                                    454 ;	-----------------------------------------
                                    455 ;	 function TIM4_GetCounter
                                    456 ;	-----------------------------------------
      008E69                        457 _TIM4_GetCounter:
                                    458 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 298: return (uint8_t)(TIM4->CNTR);
      008E69 AE 53 44         [ 2]  459 	ldw	x, #0x5344
      008E6C F6               [ 1]  460 	ld	a, (x)
      008E6D 81               [ 4]  461 	ret
                                    462 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 306: TIM4_Prescaler_TypeDef TIM4_GetPrescaler(void)
                                    463 ;	-----------------------------------------
                                    464 ;	 function TIM4_GetPrescaler
                                    465 ;	-----------------------------------------
      008E6E                        466 _TIM4_GetPrescaler:
                                    467 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 309: return (TIM4_Prescaler_TypeDef)(TIM4->PSCR);
      008E6E AE 53 45         [ 2]  468 	ldw	x, #0x5345
      008E71 F6               [ 1]  469 	ld	a, (x)
      008E72 81               [ 4]  470 	ret
                                    471 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 319: FlagStatus TIM4_GetFlagStatus(TIM4_FLAG_TypeDef TIM4_FLAG)
                                    472 ;	-----------------------------------------
                                    473 ;	 function TIM4_GetFlagStatus
                                    474 ;	-----------------------------------------
      008E73                        475 _TIM4_GetFlagStatus:
                                    476 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 324: assert_param(IS_TIM4_GET_FLAG_OK(TIM4_FLAG));
      008E73 7B 03            [ 1]  477 	ld	a, (0x03, sp)
      008E75 A1 01            [ 1]  478 	cp	a, #0x01
      008E77 27 11            [ 1]  479 	jreq	00107$
      008E79 AE 8F 0A         [ 2]  480 	ldw	x, #___str_0+0
      008E7C 4B 44            [ 1]  481 	push	#0x44
      008E7E 4B 01            [ 1]  482 	push	#0x01
      008E80 4B 00            [ 1]  483 	push	#0x00
      008E82 4B 00            [ 1]  484 	push	#0x00
      008E84 89               [ 2]  485 	pushw	x
      008E85 CD 80 A8         [ 4]  486 	call	_assert_failed
      008E88 5B 06            [ 2]  487 	addw	sp, #6
      008E8A                        488 00107$:
                                    489 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 326: if ((TIM4->SR1 & (uint8_t)TIM4_FLAG)  != 0)
      008E8A AE 53 42         [ 2]  490 	ldw	x, #0x5342
      008E8D F6               [ 1]  491 	ld	a, (x)
      008E8E 14 03            [ 1]  492 	and	a, (0x03, sp)
      008E90 4D               [ 1]  493 	tnz	a
      008E91 27 03            [ 1]  494 	jreq	00102$
                                    495 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 328: bitstatus = SET;
      008E93 A6 01            [ 1]  496 	ld	a, #0x01
      008E95 81               [ 4]  497 	ret
      008E96                        498 00102$:
                                    499 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 332: bitstatus = RESET;
      008E96 4F               [ 1]  500 	clr	a
                                    501 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 334: return ((FlagStatus)bitstatus);
      008E97 81               [ 4]  502 	ret
                                    503 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 344: void TIM4_ClearFlag(TIM4_FLAG_TypeDef TIM4_FLAG)
                                    504 ;	-----------------------------------------
                                    505 ;	 function TIM4_ClearFlag
                                    506 ;	-----------------------------------------
      008E98                        507 _TIM4_ClearFlag:
                                    508 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 347: assert_param(IS_TIM4_GET_FLAG_OK(TIM4_FLAG));
      008E98 7B 03            [ 1]  509 	ld	a, (0x03, sp)
      008E9A A1 01            [ 1]  510 	cp	a, #0x01
      008E9C 27 11            [ 1]  511 	jreq	00104$
      008E9E AE 8F 0A         [ 2]  512 	ldw	x, #___str_0+0
      008EA1 4B 5B            [ 1]  513 	push	#0x5b
      008EA3 4B 01            [ 1]  514 	push	#0x01
      008EA5 4B 00            [ 1]  515 	push	#0x00
      008EA7 4B 00            [ 1]  516 	push	#0x00
      008EA9 89               [ 2]  517 	pushw	x
      008EAA CD 80 A8         [ 4]  518 	call	_assert_failed
      008EAD 5B 06            [ 2]  519 	addw	sp, #6
      008EAF                        520 00104$:
                                    521 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 350: TIM4->SR1 = (uint8_t)(~TIM4_FLAG);
      008EAF 7B 03            [ 1]  522 	ld	a, (0x03, sp)
      008EB1 43               [ 1]  523 	cpl	a
      008EB2 AE 53 42         [ 2]  524 	ldw	x, #0x5342
      008EB5 F7               [ 1]  525 	ld	(x), a
      008EB6 81               [ 4]  526 	ret
                                    527 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 360: ITStatus TIM4_GetITStatus(TIM4_IT_TypeDef TIM4_IT)
                                    528 ;	-----------------------------------------
                                    529 ;	 function TIM4_GetITStatus
                                    530 ;	-----------------------------------------
      008EB7                        531 _TIM4_GetITStatus:
      008EB7 88               [ 1]  532 	push	a
                                    533 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 367: assert_param(IS_TIM4_IT_OK(TIM4_IT));
      008EB8 7B 04            [ 1]  534 	ld	a, (0x04, sp)
      008EBA A1 01            [ 1]  535 	cp	a, #0x01
      008EBC 27 11            [ 1]  536 	jreq	00108$
      008EBE AE 8F 0A         [ 2]  537 	ldw	x, #___str_0+0
      008EC1 4B 6F            [ 1]  538 	push	#0x6f
      008EC3 4B 01            [ 1]  539 	push	#0x01
      008EC5 4B 00            [ 1]  540 	push	#0x00
      008EC7 4B 00            [ 1]  541 	push	#0x00
      008EC9 89               [ 2]  542 	pushw	x
      008ECA CD 80 A8         [ 4]  543 	call	_assert_failed
      008ECD 5B 06            [ 2]  544 	addw	sp, #6
      008ECF                        545 00108$:
                                    546 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 369: itstatus = (uint8_t)(TIM4->SR1 & (uint8_t)TIM4_IT);
      008ECF AE 53 42         [ 2]  547 	ldw	x, #0x5342
      008ED2 F6               [ 1]  548 	ld	a, (x)
      008ED3 14 04            [ 1]  549 	and	a, (0x04, sp)
      008ED5 6B 01            [ 1]  550 	ld	(0x01, sp), a
                                    551 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 371: itenable = (uint8_t)(TIM4->IER & (uint8_t)TIM4_IT);
      008ED7 AE 53 41         [ 2]  552 	ldw	x, #0x5341
      008EDA F6               [ 1]  553 	ld	a, (x)
      008EDB 14 04            [ 1]  554 	and	a, (0x04, sp)
                                    555 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 373: if ((itstatus != (uint8_t)RESET ) && (itenable != (uint8_t)RESET ))
      008EDD 0D 01            [ 1]  556 	tnz	(0x01, sp)
      008EDF 27 06            [ 1]  557 	jreq	00102$
      008EE1 4D               [ 1]  558 	tnz	a
      008EE2 27 03            [ 1]  559 	jreq	00102$
                                    560 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 375: bitstatus = (ITStatus)SET;
      008EE4 A6 01            [ 1]  561 	ld	a, #0x01
                                    562 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 379: bitstatus = (ITStatus)RESET;
      008EE6 21                     563 	.byte 0x21
      008EE7                        564 00102$:
      008EE7 4F               [ 1]  565 	clr	a
      008EE8                        566 00103$:
                                    567 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 381: return ((ITStatus)bitstatus);
      008EE8 5B 01            [ 2]  568 	addw	sp, #1
      008EEA 81               [ 4]  569 	ret
                                    570 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 391: void TIM4_ClearITPendingBit(TIM4_IT_TypeDef TIM4_IT)
                                    571 ;	-----------------------------------------
                                    572 ;	 function TIM4_ClearITPendingBit
                                    573 ;	-----------------------------------------
      008EEB                        574 _TIM4_ClearITPendingBit:
                                    575 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 394: assert_param(IS_TIM4_IT_OK(TIM4_IT));
      008EEB 7B 03            [ 1]  576 	ld	a, (0x03, sp)
      008EED A1 01            [ 1]  577 	cp	a, #0x01
      008EEF 27 11            [ 1]  578 	jreq	00104$
      008EF1 AE 8F 0A         [ 2]  579 	ldw	x, #___str_0+0
      008EF4 4B 8A            [ 1]  580 	push	#0x8a
      008EF6 4B 01            [ 1]  581 	push	#0x01
      008EF8 4B 00            [ 1]  582 	push	#0x00
      008EFA 4B 00            [ 1]  583 	push	#0x00
      008EFC 89               [ 2]  584 	pushw	x
      008EFD CD 80 A8         [ 4]  585 	call	_assert_failed
      008F00 5B 06            [ 2]  586 	addw	sp, #6
      008F02                        587 00104$:
                                    588 ;	../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c: 397: TIM4->SR1 = (uint8_t)(~TIM4_IT);
      008F02 7B 03            [ 1]  589 	ld	a, (0x03, sp)
      008F04 43               [ 1]  590 	cpl	a
      008F05 AE 53 42         [ 2]  591 	ldw	x, #0x5342
      008F08 F7               [ 1]  592 	ld	(x), a
      008F09 81               [ 4]  593 	ret
                                    594 	.area CODE
      008F0A                        595 ___str_0:
      008F0A 2E 2E 2F 2E 2E 2F 6D   596 	.ascii "../../mipos/arch/stm8/stm8s-sdcc/src/stm8s_tim4.c"
             69 70 6F 73 2F 61 72
             63 68 2F 73 74 6D 38
             2F 73 74 6D 38 73 2D
             73 64 63 63 2F 73 72
             63 2F 73 74 6D 38 73
             5F 74 69 6D 34 2E 63
      008F3B 00                     597 	.db 0x00
                                    598 	.area INITIALIZER
                                    599 	.area CABS (ABS)
