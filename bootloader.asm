
bits	16

	org	0x500
	jmp	main

%include "inc/stdio.inc"
%include "inc/gdt.inc"
%include "inc/a20.inc"
%include "inc/vga.inc"
%include "inc/fat12.inc"
%include "inc/bootinfo.inc"
%include "inc/memory.inc"

%define NULL_DESC 0
%define CODE_DESC 0x8
%define DATA_DESC 0x10
; where the kernel is to be loaded to in protected mode
%define IMAGE_PMODE_BASE 0x40000
; where the kernel is to be loaded to in real mode
%define IMAGE_RMODE_BASE 0x3000



;*********************************************
;	Data	section
;*********************************************

LoadingMsg	db	"Preparing to load OS...",0x0d,0x0a,0x00
A20Message	db	"A20 Succeed.",0x0d,0x0a,0x00
HelloMsg	db	"Hello PM Print",0x0a,0x00
;***********************************************
;	Boot infomation:
;		include the memory infomation
;		for kernel to enable memory page management
;***********************************************
boot_info:
istruc multiboot_info
	at multiboot_info.flags,			dd 0
	at multiboot_info.memoryLo,			dd 0
	at multiboot_info.memoryHi,			dd 0
	at multiboot_info.bootDevice,		dd 0
	at multiboot_info.cmdLine,			dd 0
	at multiboot_info.mods_count,		dd 0
	at multiboot_info.mods_addr,		dd 0
	at multiboot_info.syms0,			dd 0
	at multiboot_info.syms1,			dd 0
	at multiboot_info.syms2,			dd 0
	at multiboot_info.mmap_length,		dd 0
	at multiboot_info.mmap_addr,		dd 0
	at multiboot_info.drives_length,	dd 0
	at multiboot_info.drives_addr,		dd 0
	at multiboot_info.config_table,		dd 0
	at multiboot_info.bootloader_name,	dd 0
	at multiboot_info.apm_table,		dd 0
	at multiboot_info.vbe_control_info,	dd 0
	at multiboot_info.vbe_mode_info,	dw 0
	at multiboot_info.vbe_interface_seg,dw 0
	at multiboot_info.vbe_interface_off,dw 0
	at multiboot_info.vbe_interface_len,dw 0
iend

;*********************************************
;	Bootloader entry
;		main
;*********************************************
bits	16
main:
	;--------------------------------------
	;	setup stack and segments first
	;--------------------------------------
	cli
	xor	ax,ax
	mov ds,ax
	mov es,ax
	mov ax,0x0000		;stack begins at 0x90000-0xffff
	mov ss,ax
	mov sp,0xffff
	sti
	
	mov     [boot_info+multiboot_info.bootDevice], dl
	
	;----------------------------------------
	;	show booting message
	;----------------------------------------
	mov si,LoadingMsg
	call	Puts16
	;-------------------------------------------
	;   Install our GDT
	;-------------------------------------------
	call	InstallGDT		; install our GDT
	;----------------------------------------
	;	Enable A20 Key board out
	;----------------------------------------
	call EnableA20_SysControlA
	mov si,A20Message
	call Puts16
	sti
	
	;-------------------------------------------
	;	Get memory info
	;-------------------------------------------
	xor eax,eax
	xor	ebx,ebx
	call BiosGetMemorySize64MB
	mov		word [boot_info+multiboot_info.memoryHi], bx
	mov		word [boot_info+multiboot_info.memoryLo], ax
	mov		eax, 0x0
	mov		ds, ax
	mov		di, 0x1000
	call	BiosGetMemoryMap
	;-------------------------------------------
	;	Load Kernel
	;-------------------------------------------
	call LoadRoot

	mov ebx,0
	mov ebp,IMAGE_RMODE_BASE
	mov esi,ImageName
	call LoadFile
	mov dword[ImageSize],ecx
	cmp	ax,0
	je	GoToPMode
	mov si,msgFailure
	call Puts16
	mov ah,0
	int 0x16
	int 0x19			; press any to reboot
	
GoToPMode:
	
	;----------------------------------------
	;	go to protected mode
	;----------------------------------------
	cli
	mov eax,cr0
	or	eax,1
	mov cr0,eax
	
	jmp	CODE_DESC:GoToKernel
;*******************************************************************
bits	32
GoToKernel:
	;-----------------------------------------
	;	set registers
	;-----------------------------------------
	mov ax,DATA_DESC
	mov ds,ax
	mov ss,ax
	mov es,ax
	mov esp,90000h
	
CopyImage:
	mov eax,dword[ImageSize]
	movzx ebx,word[bpbBytesPerSector]
	mul	ebx
	mov ebx,4
	div	ebx
	cld
	mov esi,IMAGE_RMODE_BASE
	mov edi,IMAGE_PMODE_BASE
	mov ecx,eax
	rep movsd
JmpToKernel:
	mov	edx, [ImageSize]
	push	dword boot_info
	call IMAGE_PMODE_BASE
	add	esp,4
	
fin:
	hlt
	jmp	fin

; kernel name (Must be 11 bytes)
ImageName     db "KRNL    SYS"
; size of kernel image in bytes
ImageSize     db 0
msgFailure	db	"Load kernel failure.",0



