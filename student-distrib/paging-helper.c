

#define SIZE_OF_PG 4096

typedef struct p_d1{
    uint32_t p   : 1;
    uint32_t rw  : 1; // set to 1
    uint32_t us  : 1;  
    uint32_t pwt : 1;
    uint32_t pcd : 1;
    uint32_t a : 1;
    uint32_t DC1 : 1;
    uint32_t ps : 1;
    uint32_t g : 1;
    uint32_t avail : 3;
    uint32_t pt_baddr : 20;
}__attribute__ ((packed)) page_dir_entry_1kb;

typedef struct p_d2{
    uint32_t p   : 1;
    uint32_t rw  : 1;
    uint32_t us  : 1;  
    uint32_t pwt : 1;
    uint32_t pcd : 1
    uint32_t a : 1;
    uint32_t DC1 : 1;
    uint32_t ps : 1;
    uint32_t g : 1;
    uint32_t avail : 3;
    uint32_t pat : 1;
    uint32_t reserv1 : 9;
    uint32_t pt_baddr : 10;
}__attribute__ ((packed)) page_dir_entry_4mb;

typedef struct p_t{
    uint32_t p   : 1;
    uint32_t rw  : 1; // set to 1
    uint32_t us  : 1;  
    uint32_t pwt : 1;
    uint32_t pcd : 1;
    uint32_t a : 1;
    uint32_t d : 1;
    uint32_t pat : 1;
    uint32_t g : 1;
    uint32_t avail : 3;
    uint32_t pt_baddr : 20;
}__attribute__ ((packed)) page_table_entry;

uint32_t page_directory[1024] __attribute__((aligned(SIZE_OF_PG)));


uint32_t page_table[1024] __attribute__((aligned(SIZE_OF_PG)));

uint32_t page __attribute__((aligned(SIZE_OF_PG))); //4kb page 

//init the page directory 
// 0x00400000 satrting addr of the kernal offset 
int i;

/*
page table base Addr 31->12: Upper 20 bits of the physical address of a 4K-aligned page table. In MP4, kernel
physical and virtual addresses are the same, so the page table base address should be just a label or variable
name from their code. All page tables must be 4K aligned, so the lower 12 bits of their addresses are 0 
*/
// set PS = 1 for 1 MB page for the kernel

// set the page dirctory 
for(i = 0; i<1024; i++){
    if( i == 0){ // set the video mem 
        page_directory[i] = page_dir_entry_1kb;
        page_directory[i].p = 1;
        page_directory[i].rw = 1; // set to 1
        page_directory[i].us = 0; // set to kernel permission 
        page_directory[i].pwt = 0;  
        page_directory[i].pcd =  0; //for vid mem     
        page_directory[i].a =  0;   // not used 
        page_directory[i].DC1 =  0;  //always set  
        page_directory[i].ps = 0;  // 4k pg table  
        page_directory[i].g =  0;   //?
        page_directory[i].avail = 0;   //dont use 
        page_directory[i].pt_baddr = 0xB8000 << 12; //shift <<12 since lower 12 bits 0 for alignment (0xB8000 –> 0xC0000) descripters.pdf pg 5
    }
    else if(i = 1){ //set the 4mb kernel page
        page_directory[i] = page_dir_entry_4mb;
        page_directory[i].p = 1;
        page_directory[i].rw = 1; // set to 1
        page_directory[i].us = 0;
        page_directory[i].pwt = 0;  
        page_directory[i].pcd =  1;    
        page_directory[i].a =  0;   
        page_directory[i].DC1 =  0;   
        page_directory[i].ps = 1;   // 4mb page table
        page_directory[i].g =  0;   // ??? check 
        page_directory[i].avail = 0;   
        page_directory[i].pt_baddr = tss_t.cr3 << 12; //cr3 
    }
    else{ //all other page table entries
        page_directory[i] = page_dir_entry_1kb;
        page_directory[i].p = 1;
        page_directory[i].rw = 1; // set to 1
        page_directory[i].us = 0;
        page_directory[i].pwt = 0;  
        page_directory[i].pcd =  1;    
        page_directory[i].a =  0;   
        page_directory[i].DC1 =  0;   
        page_directory[i].ps = 1;   // 4mb page table
        page_directory[i].g =  0;   // ??? check 
        page_directory[i].avail = 0;   
        page_directory[i].pt_baddr = 0; //cr3  
    }

}

// init the page table 
for(i = 0; i<4096; i++){
    page_directory[i] = page_table_entry;
    page_directory[i].p = 1;
    page_directory[i].rw = 1; // set to 1
    page_directory[i].us = 0; // set to kernel permission 
    page_directory[i].pwt = 0;  
    page_directory[i].pcd =  0; //for vid mem     
    page_directory[i].a =  0;   // not used 
    page_directory[i].d =  0;  //always set  
    page_directory[i].pat = 0;  // 4k pg table  
    page_directory[i].g =  0;   //?
    page_directory[i].avail = 0;   //dont use 
    page_directory[i].pt_baddr = 0;
}

void init_page(){


}

#define ldPageDir(desc)                       \
do {                                    \
    asm volatile ("ltr %w0"             \
            :                           \
            : "r" (desc)                \
            : "memory", "cc"            \
    );                                  \
} while (0)

#define enPaging(desc)                       \
do {                                    \
    asm volatile ("ltr %w0"             \
            :                           \
            : "r" (desc)                \
            : "memory", "cc"            \
    );                                  \
} while (0)