	.section	__TEXT,__text
	.globl	_main
_main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$2, %ecx
	movl	%ecx, -4(%ebp)
	movl	$1, %ecx
	movl	%ecx, -8(%ebp)
	movl	-4(%ebp), %eax
	movl	-8(%ebp), %ecx
	cmpl	%ecx, %eax
	jle	.L0
	movl	-8(%ebp), %eax
	movl	$1, %ecx
	addl	%ecx, %eax
	movl	%eax, -4(%ebp)
.L0:
	movl	-8(%ebp), %eax
	movl	-4(%ebp), %ecx
	cmpl	%ecx, %eax
	jne	.L2
	movl	-4(%ebp), %eax
	movl	-8(%ebp), %ecx
	addl	%ecx, %eax
	movl	%eax, -4(%ebp)
	jmp	.L1
.L2:
	movl	-4(%ebp), %eax
	movl	-8(%ebp), %ecx
	subl	%ecx, %eax
	movl	%eax, -4(%ebp)
.L1:
	subl	$16, %esp
	movl	%ecx, 8(%esp)
	movl	%edx, 4(%esp)
	movl	-4(%ebp), %eax
	movl	%eax, 0(%esp)
	call	put_int
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
	addl	$16, %esp
_END_main:
	leave
	ret

	.section	__TEXT,__cstring
.LC0:
	.string "%d\n"
	.section	__TEXT,__text
put_int:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24,%esp
	calll	L0$pb
L0$pb:
	popl	%eax
	movl	8(%ebp),%ecx
	movl	%ecx, 4(%esp)
	leal	.LC0-L0$pb(%eax), %eax
	movl	%eax, (%esp)
	calll	_printf
	leave
	ret
