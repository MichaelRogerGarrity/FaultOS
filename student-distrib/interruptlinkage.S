# interruptlinkage.S - Set up a macro for the interrupt linkage - convers normal handlers to ASM wrapped handlers.
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"


#define INTR_LINK(name, func)    \
.GLOBL name                     ;\
name:                           ;\
PUSHAL	                        ;\
pushfl                          ;\
call func                       ;\
popfl                           ;\
POPAL                           ;\
IRET

INTR_LINK(rtc_handler_linkage, rtc_handler);

INTR_LINK(keyboard_handler_function, keyboard_handler);

