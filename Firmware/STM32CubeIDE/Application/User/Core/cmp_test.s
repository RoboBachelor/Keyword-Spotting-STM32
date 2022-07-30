
.syntax unified

.text

.global cmp_loop

cmp_loop:
		push	{r8-r11}
		
		
		
		/* ----------------------------
		
		;mov		r8, #6
;		ldr		r9, [r0]
;loop1
;		sub		r9, r9, #1
;		cmp		r9, #6
;		bgt		loop1


;		str		r9, [r0]
;		mov		r0, r8
		
		;---------------------------- */
		
loop3:
		
		ldr		r9, = 10240
		ldr		r8, = 0xC0000000
		
		
loop2:
		;ldr		r10, [r8], #20
		vmla.f32	s2, s6, s4
		subs	r9, r9, #1
		bgt		loop2
		
		subs	r0, r0, #1
		bgt		loop3
		
		mov		r0, r8
		
		pop		{r8-r11}
		bx		lr
