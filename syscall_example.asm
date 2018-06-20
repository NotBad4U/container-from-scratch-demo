section	.text
   global _start    ;must be declared for linker (ld)
	
_start:
   mov	eax, 4      ; system call number (sys_write)	
   mov	ebx, 1      ; file descriptor (stdout)
   mov	ecx, msg    ; message to write
   mov	edx, 16     ; message length
   int	0x80        ; call kernel

   mov	eax,1       ;system call number (sys_exit)
   int	0x80        ;call kernel
	
section	.data
msg db 'Hello voxxed LU', 0xa  ;string to be printed
