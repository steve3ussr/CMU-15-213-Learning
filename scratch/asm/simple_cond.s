	.file	"simple_cond.c"
	.text
	.globl	func
	.def	func;	.scl	2;	.type	32;	.endef
	.seh_proc	func
func:
	.seh_endprologue
	cmpl	%edx, %ecx  # ecx=a, ecx=b
	jle	.L2  # jump when a<=b
	leal	(%rcx,%rdx), %eax  # result=a+b
.L1:
	ret
.L2:
	movl	%ecx, %eax  # result=a
	subl	%edx, %eax  # result=result-b, result = a-b
	jmp	.L1
	.seh_endproc
	.ident	"GCC: (x86_64-posix-seh-rev0, Built by MinGW-W64 project) 8.1.0"
