	.text
	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	-4(%ebp), %eax
	movl	-8(%ebp), %ecx
	addl	%ecx, %eax
	jmp	_END_main
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
	call	printf
	leave
	ret
