	.section	__TEXT,__text
	.globl	sum
sum:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movl	12(%ebp), %ecx
	subl	%ecx, %eax
	jmp	_END_sum
_END_sum:
	leave
	ret

	.globl	_main
_main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	subl	$24, %esp
	movl	%eax, 16(%esp)
	movl	%edx, 8(%esp)
	movl	$2, %eax
	movl	%eax, 4(%esp)
	movl	$1, %eax
	movl	%eax, 0(%esp)
	call	sum
	movl	%eax, %ecx
	movl	16(%esp), %eax
	movl	8(%esp), %edx
	addl	$24, %esp
	movl	%ecx, -4(%ebp)
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
