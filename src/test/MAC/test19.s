	.section	__TEXT,__text
	.globl	_main
_main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$1, %ecx
	movl	%ecx, -4(%ebp)
	movl	$2, %ecx
	movl	%ecx, -8(%ebp)
	movl	$3, %ecx
	movl	%ecx, -12(%ebp)
	movl	$4, %ecx
	movl	%ecx, -16(%ebp)
	movl	$5, %ecx
	movl	%ecx, -20(%ebp)
	movl	$6, %ecx
	movl	%ecx, -24(%ebp)
	movl	-4(%ebp), %eax
	movl	-8(%ebp), %ecx
	addl	%ecx, %eax
	movl	-12(%ebp), %ecx
	movl	-16(%ebp), %edx
	addl	%edx, %ecx
	addl	%ecx, %eax
	movl	-20(%ebp), %ecx
	movl	-24(%ebp), %edx
	addl	%edx, %ecx
	addl	%ecx, %eax
	movl	%eax, -4(%ebp)
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
