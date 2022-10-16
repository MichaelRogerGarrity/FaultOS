
    #define INTR_LINK(name, func)   \
    .GLOBL name                   ;\
    name:  ;\  
    PUSHAL	          ;\
    pushfl ;\
	call func  ;\
    popfl ;\
	POPAL  ;\
	IRET

    INTR_LINK(rtc_handler_linkage, rtc_handler);

    INTR_LINK(keyboard_handler_linkage, keyboard_handler);

