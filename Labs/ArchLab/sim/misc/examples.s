	.file	"examples.c"
	.text
	.globl	sum_list
	.type	sum_list, @function
sum_list:  # rdi is a list_pointer
.LFB0:
	.cfi_startproc
	movl	$0, %eax  # val
	jmp	.L2  # check cond first
.L3:
	addq	(%rdi), %rax  # val += ls.val
	movq	8(%rdi), %rdi # ls = ls+0x8
.L2:
	testq	%rdi, %rdi  # exec loop if rdi!=0
	jne	.L3
	ret
	.cfi_endproc
.LFE0:
	.size	sum_list, .-sum_list
	.globl	rsum_list
	.type	rsum_list, @function
rsum_list:
.LFB1:
	.cfi_startproc
	testq	%rdi, %rdi
	je	.L6
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	(%rdi), %rbx
	movq	8(%rdi), %rdi
	call	rsum_list
	addq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
.L6:
	.cfi_restore 3
	movl	$0, %eax
	ret
	.cfi_endproc
.LFE1:
	.size	rsum_list, .-rsum_list
	.globl	copy_block
	.type	copy_block, @function
copy_block:
.LFB2:
	.cfi_startproc
	movl	$0, %ecx
	jmp	.L12
.L13:
	movq	(%rdi), %rax
	movq	%rax, (%rsi)
	xorq	%rax, %rcx
	subq	$1, %rdx
	leaq	8(%rsi), %rsi
	leaq	8(%rdi), %rdi
.L12:
	testq	%rdx, %rdx
	jg	.L13
	movq	%rcx, %rax
	ret
	.cfi_endproc
.LFE2:
	.size	copy_block, .-copy_block
	.globl	main
	.type	main, @function
main:
.LFB3:
	.cfi_startproc
	movl	$0, %eax
	ret
	.cfi_endproc
.LFE3:
	.size	main, .-main
	.ident	"GCC: (Debian 12.2.0-14) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
