    # #define INTR_LINK(name, func)   \
    # .GLOBL name                   ;\
    # name:   PUSHAL	          ;\
	# call func  ;\
	# POPAL  ;\
	# IRET

    # INTR_LINK(rtc_handler_linkage, rtc_handler);