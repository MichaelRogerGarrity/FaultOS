

# NUM_SYS_CALLS: 10

# before calling:
#	 %EAX: Call Number
#	 %EBX: Arg 1
#	 %ECX: Arg 2
#	 %EDX: Arg 3
.text
 .globl call_handler
call_handler:
	cmpl $10, %eax
	ja badcode
	addl $-1, %eax 	# jump_table is zero-indexed
	pushfl
	pushl %edx
	pushl %ecx
	pushl %ebx
	call *jump_table(,%eax,4)
	popl %ebx
	popl %ecx
	popl %edx
	popfl

	jmp done

jump_table:
	.long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

badcode:
	movl $-1, %eax	# return error
done:
	iret
