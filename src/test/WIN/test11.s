	.text
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

	.section	.rodata
.LC0:
	.string "%d\n"
	.text
put_int:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24,%esp
	movl	$.LC0, %eax
	movl	8(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	_printf
	leave
	ret
