#include "paging.h"

 page_dir_entry page_directory[1024] __attribute__((aligned(SIZE_OF_PG)));
 page_table_entry page_table[1024] __attribute__((aligned(SIZE_OF_PG)));
 void init_page();
//uint32_t page __attribute__((aligned(SIZE_OF_PG))); //4kb page 

//init the page directory 

int i;

/*
page table base Addr 31->12: Upper 20 bits of the physical address of a 4K-aligned page table. In MP4, kernel
physical and virtual addresses are the same, so the page table base address should be just a label or variable
name from their code. All page tables must be 4K aligned, so the lower 12 bits of their addresses are 0 
*/
// set PS = 1 for 1 MB page for the kernel
 void init_page(){
// set the page dirctory 
    for(i = 0; i<1024; i++){
    // if( i == 0){ // set the video mem 
            //page_directory[i] = page_dir_entry_1kb;
        page_directory[i].p = 0;
        page_directory[i].rw = 1; // set to 1
        page_directory[i].us = 0; // set to kernel permission 
        page_directory[i].pwt = 0;  
        page_directory[i].pcd =  0; //for vid mem     
        page_directory[i].a =  0;   // not used 
        page_directory[i].DC1 =  0;  //always set  
        page_directory[i].ps = 0;  // 4k pg table  
        page_directory[i].g =  0;   //?
        page_directory[i].avail = 0;   //dont use 
        page_directory[i].pt_baddr = 0; //(uint32_t) 0xB8000 >> 12; //shift <<12 since lower 12 bits 0 for alignment (0xB8000 –> 0xC0000) descripters.pdf pg 5
        // }
    // else{ //all other page table entries
        page_table[i].p = 0;
        page_table[i].rw = 1; // set to 1
        page_table[i].us = 0;
        page_table[i].pwt = 0;  
        page_table[i].pcd =  0;    
        page_table[i].a =  0;   
        page_table[i].d =  0;   
        page_table[i].pat = 0;   // 4mb page table
        page_table[i].g =  0;   // ??? check 
        page_table[i].avail = 0;   
        page_table[i].pt_baddr = 0; //cr3  
        //}
    }
    
    //set up the kernel 
    page_directory[1].p = 1;
    page_directory[1].ps = 1;   // 4mb page table
    //page_directory[1].g = 1;   // 4mb page table
    //page_directory[1].pcd =  1; 
    page_directory[1].rw = 1;
    // what else? page_directory[0].us = 0; // set to kernel permission 
    page_directory[1].pt_baddr = (uint32_t)(KERNEL_ADDR) >> 12; //x1 << 10 

    //set the vid mem 
    page_directory[0].p = 1;
    page_directory[0].rw = 1;
    // what else? page_directory[0].us = 0; // set to kernel permission 
    page_directory[0].pt_baddr = (int)(page_table) >> 12;

    
    // Setting Video Memory inside the page table
    for(i = 0; i<1024; i++){
        page_table[i].pt_baddr = i;
        
        if( i==0xb8){
            page_table[i].p = 1; 
            page_table[i].pt_baddr = 0xb8;
        }
    }

    //page_table[h].pt_baddr = (uint32_t)(VIDEO) >> 12; 


    loadPageDir(page_directory);
    enPaging();

}//end of init 












// // init the page table 
// for(i = 0; i<4096; i++){
//     page_directory[i] = page_table_entry;
//     page_directory[i].p = 1;
//     page_directory[i].rw = 1; // set to 1
//     page_directory[i].us = 0; // set to kernel permission 
//     page_directory[i].pwt = 0;  
//     page_directory[i].pcd =  0; //for vid mem     
//     page_directory[i].a =  0;   // not used 
//     page_directory[i].d =  0;  //always set  
//     page_directory[i].pat = 0;  // 4k pg table  
//     page_directory[i].g =  0;   //?
//     page_directory[i].avail = 0;   //dont use 
//     page_directory[i].pt_baddr = 0;
// }



