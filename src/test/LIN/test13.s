	.text
	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$1, %ecx
	movl	%ecx, -4(%ebp)
	movl	$2, %ecx
	movl	%ecx, -8(%ebp)
	movl	-4(%ebp), %eax
	movl	-8(%ebp), %ecx
	cmpl	%ecx, %eax
	setg	%al
	movzbl	%al, %eax
	movl	%eax, -12(%ebp)
	subl	$16, %esp
	movl	%ecx, 8(%esp)
	movl	%edx, 4(%esp)
	movl	-12(%ebp), %eax
	movl	%eax, 0(%esp)
	call	put_int
	movl	8(%esp), %ecx
	movl	4(%esp), %edx
	addl	$16, %esp
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
