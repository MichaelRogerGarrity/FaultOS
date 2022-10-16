# alloc the space for the directory and table
#enable the paging after we have inialized the page tables (bit 31 in CR0 reg)

#    .align 4096
#page_directory:  
#    .rept 1024
#    .long 0    #init 1024 32-bit entireis in page directory  
#    .endr

#    .align 4096
#page_table:  
#    .rept 1024
#    .long 0    #init 1024 32-bit entireis in page table  
#    .endr

#    .align 4096
#page:  
#    .rept 1024
#    .long 0    #init a 1 kb page   
#   .endr

# set PS = 1 pde points directly to 4MB page for kernel 




#enPaging:

