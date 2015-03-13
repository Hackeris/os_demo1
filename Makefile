all:
	nasm boot.asm -o boot.bin
	nasm bootloader.asm -o krnldr.sys
krnl:
	pe2bin.exe krnl.sys
