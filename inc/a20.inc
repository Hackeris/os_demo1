
bits	16

;******************************************
;	Enable A20 address line.
;******************************************


%ifndef __A20_INC_67343546FDCC56AAB872_INCLUDED__
%define __A20_INC_67343546FDCC56AAB872_INCLUDED__

;******************************************
; Enables a20 line through keyboard controller
;******************************************

EnableA20_KKbrd:
	cli
	push	ax
	mov al,0xdd
	out	0x64,al
	pop	ax
	ret
	
;********************************************
; 	Enables a20 line through output port
;********************************************

EnableA20_KKbrd_Out:
	cli
	pusha
		call wait_input
		mov al,0xad			;disable keyboard
		out	0x64,al
		call wait_input
		
		mov al,0xd0				;tell controller to read output port
		out	0x64,al
		call wait_output
		
		in al,0x60
		push eax				;store output data
		call wait_input
		
		mov al,0xd1				;tell the controller to write output port
		out 0x64,al
		call wait_input
		
		pop	eax
		or	al,2
		out	0x60,al
		call wait_input
		
		mov al,0xae				;enable keyboard
		out	0x64,al
		
		call wait_input
	
	popa
		sti
		ret
wait_input:
	in	al,0x64
	test al,2
	jnz	wait_input
	ret
wait_output:
	in	al,0x64
	test al,1
	jz	wait_output
	ret
	
;******************************************
;	Enable A20 address line 
;		Using System control port A
;******************************************

EnableA20_SysControlA:
	push	ax
	mov al,2
	out	0x92,al
	pop ax
	ret
	
;******************************************
;	Enable A20 address line
;		Through	BIOS
;******************************************

EnableA20_BIOS:
	pusha
	mov ax,0x2401
	int 0x15
	popa
	ret

%endif