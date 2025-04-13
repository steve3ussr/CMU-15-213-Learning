	.file	"switch_jump_table.c"
	.text
	.globl	func
	.def	func;	.scl	2;	.type	32;	.endef
	.seh_proc	func
func:
	.seh_endprologue
	cmpl	$6, %ecx
	ja	.L2
	movl	%ecx, %ecx
	leaq	.L4(%rip), %rdx
	movslq	(%rdx,%rcx,4), %rax
	addq	%rdx, %rax
	jmp	*%rax
	.section .rdata,"dr"
	.align 4
.L4:
	.long	.L2-.L4
	.long	.L8-.L4
	.long	.L9-.L4
	.long	.L6-.L4
	.long	.L2-.L4
	.long	.L10-.L4
	.long	.L3-.L4
	.text
.L3:
	movl	$0, %eax
	jmp	.L5
.L8:
	movl	$101, %eax
	jmp	.L1
.L6:
	movl	$103, %eax
	jmp	.L1
.L10:
	movl	$105, %eax
.L5:
	addl	$1, %eax
.L1:
	ret
.L2:
	movl	$1000, %eax
	jmp	.L1
.L9:
	movl	$102, %eax
	jmp	.L1
	.seh_endproc
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0"
