	.text
	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$3, %ecx
	movl	%ecx, -8(%ebp)
	movl	-8(%ebp), %eax
	movl	$1, %ecx
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
	movl	-8(%ebp), %eax
	movl	$2, %ecx
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
