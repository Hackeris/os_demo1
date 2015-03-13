%define	VIDMEM	0xb8000
%define COLS	80
%define LINES	25
%define	CHAR_ATTRIB	0x7
CurX	db	0
CurY	db	0

bits	32
;*********************************************************
;	Puch32:
;		put the char in bl to screen
;*********************************************************
Putch32:
	pusha					; save all registers
	mov edi,VIDMEM
	
	xor	eax,eax				; clear eax
	mov ecx,COLS*2			; Mode 7 has 2 bytes per char, so its COLS*2 bytes per line
	mov al,byte[CurY]		; get y pos
	mul	ecx					; eax=CurY*COLS*2
	push	eax
	
	xor	eax,eax				; clear eax
	mov al,byte[CurX]		; get x pos
	mov cl,2
	mul	cl					; eax=eax*cl
	pop	ecx
	add	eax,ecx				; eax=(x+y*cols)*2
	
	xor	ecx,ecx
	add	edi,eax				;	edi = edi+eax
	
	;	is a next line?
	cmp	bl,0x0a
	je	.row
	
	mov dl,bl				; the word is in bl
	mov dh,CHAR_ATTRIB		; the attribute of word
	mov word[edi],dx		; display the char
	
	inc byte[CurX]
	cmp	word[CurX],COLS
	je	.row
	jmp	.done
.row:
	mov byte[CurX],0
	inc byte[CurY]
.done:
	popa
	ret
;*************************************************
;	Puts32:
;		put a 0 terminated string whose pointer is in ebx to screen
;*************************************************
Puts32:
	pusha					; store registers
	push	ebx
	pop	edi					; copy the string address from ebx to edi
.loop:
	mov bl,byte[edi]
	cmp	bl,0
	je	.done
	call	Putch32
.next:
	inc	edi
	jmp	.loop
.done:
	mov bh,byte[CurY]
	mov bl,byte[CurX]
	call	MovCur
	popa
	ret

;*********************************************************
;	MovCur:
;		Update the system cursor
;		Move the cursor to x,y
;	param:
;		bl = x ; bh = y
;*********************************************************
MovCur:
	pusha
	;----------------------------------------------
	;	Get current position.
	;----------------------------------------------
	xor	eax,eax
	mov ecx,COLS
	mov al,bh
	mul	ecx
	add	al,bl
	mov ebx,eax
	;------------------------------------------------
	;	Set low byte index to VGA register
	;------------------------------------------------
	mov al,0x0f				; Cursor location low byte index
	mov dx,0x03d4			; Write it to the CRT index register
	out	dx,al
	
	mov al,bl				; The current location is in EBX,BL contains the low byte,BH high byte
	mov dx,0x03d5			; Write it to the data register
	out dx,al				; low byte
	
	xor eax,eax
	
	mov al,0x0e				; Cursor location high byte index
	mov dx,0x03d4			; Write to the CRT index register
	out dx,al
	
	mov al,bh				; the current location is in EBX,BH contains the high byte
	mov dx,0x3d5			; Write it to the CRT data register
	out dx,al				; high byte
	
	popa
	ret
	
;********************************************************
;	ClrScr32:
;		Clear screen
;********************************************************
ClrScr32:
	pusha
	cld
	mov edi,VIDMEM
	mov cx,2000
	mov ah,CHAR_ATTRIB
	mov al,' '
	rep	stosw
	
	mov byte[CurX],0
	mov byte[CurY],0
	popa
	ret