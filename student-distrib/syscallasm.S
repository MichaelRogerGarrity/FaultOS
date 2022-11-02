#include "sysnum.h"
#include "syscall.h"
#include "x86_desc.h"


NUM_SYS_CALLS: 10

# before calling:
#	 %EAX: Call Number
#	 %EBX: Arg 1
#	 %ECX: Arg 2
#	 %EDX: Arg 3
 .globl call_handler
call_handler:
	cmpl %eax, NUM_SYS_CALLS
	ja badcode
	addl %eax, $-1	# jump_table is zero-indexed
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
	mov %eax, $-1	# return error
done:
	IRET
