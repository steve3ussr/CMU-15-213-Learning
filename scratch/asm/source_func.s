	.file	"source_func.c"
	.text
	.globl	mul2
	.def	mul2;	.scl	2;	.type	32;	.endef
	.seh_proc	mul2
mul2:
	.seh_endprologue
	leal	(%rcx,%rdx), %eax
	ret
	.seh_endproc
	.globl	multstore
	.def	multstore;	.scl	2;	.type	32;	.endef
	.seh_proc	multstore
multstore:
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%r8, %rbx
	call	mul2
	addl	$1, %eax
	movl	%eax, (%rbx)
	addq	$32, %rsp
	popq	%rbx
	ret
	.seh_endproc
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0"
