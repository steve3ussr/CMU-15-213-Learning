	.file	"sim_buf_of.c"
	.text
	.globl	gets
	.type	gets, @function
gets:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	movq	%rdi, %rbp
	movq	%rdi, %rbx
	jmp	.L2
.L4:
	movb	%al, (%rbx)
	leaq	1(%rbx), %rbx
.L2:
	movq	stdin(%rip), %rdi
	call	getc@PLT
	cmpl	$10, %eax
	je	.L3
	cmpl	$-1, %eax
	jne	.L4
.L3:
	cmpl	$-1, %eax
	sete	%dl
	cmpq	%rbp, %rbx
	sete	%al
	testb	%al, %dl
	jne	.L6
	movb	$0, (%rbx)
	movq	%rbp, %rax
.L1:
	addq	$8, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
.L6:
	.cfi_restore_state
	movl	$0, %eax
	jmp	.L1
	.cfi_endproc
.LFE11:
	.size	gets, .-gets
	.globl	main
	.type	main, @function
main:
.LFB12:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	leaq	12(%rsp), %rbx
	movq	%rbx, %rdi
	call	gets
	movq	%rbx, %rdi
	call	puts@PLT
	movl	$0, %eax
	addq	$16, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE12:
	.size	main, .-main
	.ident	"GCC: (Debian 12.2.0-14) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
