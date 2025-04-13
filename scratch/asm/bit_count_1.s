	.file	"bit_count_1.c"
	.text
	.section .rdata,"dr"
.LC0:
	.ascii "There are %d 1-bit in x\12\0"
	.text
	.globl	do_while
	.def	do_while;	.scl	2;	.type	32;	.endef
	.seh_proc	do_while
do_while:
	subq	$40, %rsp
	.seh_stackalloc	40
	.seh_endprologue
	movl	$0, %edx
.L2:
	movl	%ecx, %eax
	shrl	$31, %eax
	addl	%eax, %edx
	addl	%ecx, %ecx
	jne	.L2
	leaq	.LC0(%rip), %rcx
	call	printf
	nop
	addq	$40, %rsp
	ret
	.seh_endproc
	.globl	do_while_goto
	.def	do_while_goto;	.scl	2;	.type	32;	.endef
	.seh_proc	do_while_goto
do_while_goto:
	subq	$40, %rsp
	.seh_stackalloc	40
	.seh_endprologue
	movl	$0, %edx
.L4:
	movl	%ecx, %eax
	shrl	$31, %eax
	addl	%eax, %edx
	addl	%ecx, %ecx
	jne	.L4
	leaq	.LC0(%rip), %rcx
	call	printf
	nop
	addq	$40, %rsp
	ret
	.seh_endproc
	.def	__main;	.scl	2;	.type	32;	.endef
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
	subq	$40, %rsp
	.seh_stackalloc	40
	.seh_endprologue
	call	__main
	movl	$-252645309, %ecx
	call	do_while
	movl	$-252645309, %ecx
	call	do_while_goto
	movl	$0, %eax
	addq	$40, %rsp
	ret
	.seh_endproc
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0"
	.def	printf;	.scl	2;	.type	32;	.endef
