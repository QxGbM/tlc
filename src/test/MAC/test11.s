	.section	__TEXT,__text
	.globl	_main
_main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	-12(%ebp), %eax
	movl	-16(%ebp), %ecx
	addl	%ecx, %eax
	movl	-8(%ebp), %ecx
	addl	%eax, %ecx
	movl	-4(%ebp), %eax
	addl	%ecx, %eax
	movl	%eax, -4(%ebp)
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
