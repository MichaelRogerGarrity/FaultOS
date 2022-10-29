#include "syscall.h"
#include "x86_desc.h"
#include "IDT.h"
#include "lib.h"
#include "linkageheader.h"
#include "filesys.h"
#include "paging.h"

/* Variables for the different system calls */

static int currpid = 0;


/* System Call Functions */

// Halt: System Call Number 1
/*
int halt(uint8_t status)
Description:
Inputs:
Outputs:
*/
int32_t halt(uint8_t status)
{
    return 0;
}

// Execute: System Call Number 2
/*
int execute(const uint8_t* command)
Description:
Inputs:
Outputs:
*/
int32_t execute(const uint8_t *command)
{

    /* Order of stuff to write:

    1. Extract name and args - check whether executable
    2. Search file system for the name of the file given                // read dentry by name
    3. Extract all information about the file - if it's an executable   // read data
    4. Paging: Use PID to decide 8 + (PID*4MB)
                Then copy the whole file into that virtual address      // read data

    5. Set up PCB stuff too - create new PCB
                Set up the FD (1, 2 for terminal, rest empty)\

    6. Set up context switch ( kernel stack base into esp of tss )
    7. IRET
    */

    /*  1. Extract name and args - check whether executable  */

    int cmd_len = strlen((const uint8_t *)command);
    uint32_t filename[32];
    uint32_t arg0[128];
    // uint32_t arg1[32];
    // uint32_t arg2[32];
    uint32_t buffer[4];
    uint32_t offset = 0;
    int i = 0, argflag = -1;
    int filenamechar = 0, arg0char = 0;
    // int arg1char = 0, arg2char = 0;

    // Clear all arguments / name insert null:

    for (i = 0; i < 128; i++) {      // change
        if (i < 32)
            filename[i] = '\0';
        arg0[i] = '\0';
        // arg1[i] = '\0';
        // arg2[i] = '\0';
    }

    for (i = 0; i < cmd_len; i++) {
        if (command[i] == ' ') {
            argflag++;
            continue;
        }
        switch (argflag) {

            case -1:
            if (filenamechar < 32)
                filename[filenamechar++] = command[i];
            else
                return -1;
            break;

            case 0:
            if (arg0char < 128)// change
                arg0[arg0char++] = command[i];
            else
                return -1;
            break;

            // case 1:
            // if (arg1char < 32)
            //     arg1[arg1char++] = command[i];
            //  else
            //     return -1;
            // break;

            // case 2:
            // if (arg2char < 32)
            //     arg2[arg2char++] = command[i];
            //  else
            //     return -1;
            // break;

        } 

    }   // end of arg parsing

    /* 2. Search file system for the name of the file given                // read dentry by name */
    dentry_t currdentry;
    // Prototype: int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry)

    if (strlen(filename) == 0 || read_dentry_by_name(filename, &currdentry) < 0 )
        return -1;              // Invalid filename / could not find file
    
    /* 3. Extract all information about the file - if it is executable                          // read data 

    The layout of executable files in the file system is simple: the entire file stored in the file system is the image of the
    program to be executed. In this file, a header that occupies the first 40 bytes gives information for loading and starting
    the program. The first 4 bytes of the file represent a “magic number” that identifies the file as an executable. These
    bytes are, respectively, 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46. If the magic number is not present, the execute system
    call should fail.
    */

    int exec = 0; // not an executable
    // Prototype: int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    // Check offset and buffer size, numbytes
    if (read_data(currdentry.inode, 0, buffer, 4) < 0 ) //dont do &buffer 
        return -1;              // Invalid filename / could not find file
    if (buffer[0] == 0x7f && buffer[1] == 0x45 && buffer[3] == 0x4c && buffer[4] == 0x46) // magic numbers for executables
        exec = 1;
    if (!exec) 
        return -1;              // it is not executable

    /* 4. Paging: Use PID to decide 8 + (PID*4MB)
                Then copy the whole file into that virtual address      // read data
    */
    // Future checkpoints: make an array of structs. Right now, just use a single pid thing. 
    currpid++;                  // New process is active.


    uint32_t physaddr = (PDE_PROCESS_START + currpid) * FOUR_MB;
    page_directory[PDE_VIRTUAL_MEM].p = 1;         
    page_directory[PDE_VIRTUAL_MEM].pt_baddr = physaddr >> PAGE_SHIFT;
    page_directory[PDE_VIRTUAL_MEM].g = 0;              // Want page to be flushed when tlb is flushed
    page_directory[PDE_VIRTUAL_MEM].ps = 1;             // make it a 4 mb page
    page_directory[PDE_VIRTUAL_MEM].pcd = 1;            // in desc.pdf
    page_directory[PDE_VIRTUAL_MEM].us = 1;             // must be 1 for all user-level pages and mem ranges             
    
    loadPageDir(page_directory); //flush TLB
    // copy info into the 4mb page
    uint8_t *addrptr = (uint8_t *)(VIRT_ADDR); // PASSED INTO READ DATA AS BUFFER
    uint32_t currdentryinodenum = currdentry.inode;
    uint32_t currdentryinodelen = ((inode_t *)(inodeptr + currdentryinodenum))->length;
    // Prototype: int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    // Check offset and buffer size, numbytes
    
    if (read_data(currdentryinodenum, 0, addrptr, currdentryinodelen) < 0 )
        return -1;

    /* 5. Set up PCB stuff too - create new PCB */
    
    pcb_t * currpcb; //make global ?
    currpcb =  (uint32_t *)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1)));
    
    currpcb->pid = currpid;
    if (currpid = 0){
        currpcb-> parent_pid = -1; // check what parent of shell should be 
    }
    else{
        currpcb-> parent_pid = currpid - 1;
    }
    register uint32_t save_esp = asm("esp");   // CHECK  
    register uint32_t save_ebp = asm("ebp");     
    
    currpcb->saved_esp = save_esp; //
    currpcb->saved_ebp = save_ebp; //
    
    /*  Set up the FD (1, 2 for terminal, rest empty) */

    for(i = 0; i<MAX_FD_LEN; i++){//maybe init the entry 0 and 1 for stdin and stdout
        currpcb->(fdarray[i].fileop)->open = 0;
        currpcb->(fdarray[i].fileop)->read = 0;
        currpcb->(fdarray[i].fileop)->write = 0;
        currpcb->(fdarray[i].fileop)->close = 0;

        currpcb->fdarray[i].inode = -1;
        currpcb->fdarray[i].filepos = 0;
        currpcb->fdarray[i].present = 0;
        currpcb->fdarray[i].type = -1;
        //f2,f3 reserved (not used for now)
        currpcb->fdarray[i].f2 = -1;
        currpcb->fdarray[i].f3 = -1;
    }
    currpcb->active = 1;

    // std in
    currpcb->(fdarray[0].fileop)->open = &open_terminal;
    currpcb->(fdarray[0].fileop)->read = &read_terminal;
    currpcb->(fdarray[0].fileop)->write = NULL;
    currpcb->(fdarray[0].fileop)->close = &close_terminal;
    currpcb->fdarray[0].present = 1;

    // std out
    currpcb->(fdarray[1].fileop)->open = &open_terminal;
    currpcb->(fdarray[1].fileop)->read = NULL;
    currpcb->(fdarray[1].fileop)->write = &write_terminal;
    currpcb->(fdarray[1].fileop)->close = &close_terminal;
    currpcb->fdarray[1].present = 1;

    /* 6. Set up context switch ( kernel stack base into esp of tss ) */

    uint32_t currksp = (uint32_t)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * currpid));
    tss.ss0 = KERNEL_DS;
    tss.esp0 = currksp;

    
    return 0;
}

// Read: System Call Number 3
/*
int read(int32_t fd, void* buf, int32_t nybytes)
Description: reads data from keyboard , file, device(RTC), or directory
Inputs:
Outputs:
*/
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    /* sanity check: initial file position at eof or beyonf end of curr file */
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL || nbytes < 0) {return -1;}

    /* use a jmptable referenced by the tasks files array...
    which calls a generic handler for the specific file type's specific read function */
    if(fd == 1) return -1;  // This is terminal_write

    if((fd == 0)) {
        return terminal_read(fd, buf, nbytes);
    }

    pcb_t * currpcb; 
    currpcb = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1));

    return *(currpcb->(fdarray[fd].fileop)->read)(fd, buf, nbytes); // not sure how to call function

}

// Write: System Call Number 4
/*
int write(int32_t fd, void* buf, int32_t nbytes)
Description: writes data to either the terminal or to a device (RTC), depending on the fd
Inputs:
Outputs:
*/
int32_t write(int32_t fd, void *buf, int32_t nbytes)
{
    // nbytes_written = 0;
    /* sanity check: if device(RTC), else we write to terminal */
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL || nbytes < 0) return -1;
    /* if RTC: syscall should always accept a 4-byte int specifyinng the interrupt rate in Hz (should set the rate of periodic interuppts accordingly) */
        if(fd == 0) return -1;  // This is terminal_read

    // only does things with rtc and terminal
    if((fd == 1)) {
        return terminal_write(fd, buf, nbytes);
    }

    pcb_t * currpcb; 
    currpcb = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1));

    return *(currpcb->(fdarray[fd].fileop)->write)(fd, buf, nbytes); // not sure how to call function

    // return nbytes_written;
}

// Open: System Call Number 5
/*
int open(const uint8_t* filename)
Description:
Inputs:
Outputs:
*/
int32_t open(const uint8_t *filename)
{
    /* find the dir entry corresponding to the named file */
    if(filename == NULL) return -1;

    int dentry_name_return;
    pcb_t * currpcb; 
    currpcb = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1));

    dentry_t *currdentry;
    dentry_name_return = read_dentry_by_name(filename, currdentry);
    if(dentry_name_return == -1) return -1; // check if file exists
    int fd = -1;

     for(i = 2; i<MAX_FD_LEN; i++){
        if(!currpcb->fdarray[i].present){
            fd = i;
            break;
        } 
    }

    if(fd == -1) return -1; // fd array is full

    if((currdentry->ftype < 0) || (currdentry->ftype > 2)) return -1;
    
    else if(currdentry->ftype == 0){    // rtc
        currpcb->(fdarray[fd].fileop)->open = &open_rtc;
        currpcb->(fdarray[fd].fileop)->read = &read_rtc;
        currpcb->(fdarray[fd].fileop)->write = &write_rtc;
        currpcb->(fdarray[fd].fileop)->close = &close_rtc;
    }
    else if(currdentry->ftype == 1){    // dir
        currpcb->(fdarray[fd].fileop)->open = &open_dir;
        currpcb->(fdarray[fd].fileop)->read = &read_dir;
        currpcb->(fdarray[fd].fileop)->write = &write_dir;
        currpcb->(fdarray[fd].fileop)->close = &close_dir;
    }
    else if(currdentry->ftype == 2){    // normal file
        currpcb->(fdarray[fd].fileop)->open = &open_file;
        currpcb->(fdarray[fd].fileop)->read = &read_file;
        currpcb->(fdarray[fd].fileop)->write = &write_file;
        currpcb->(fdarray[fd].fileop)->close = &close_file;
    }

    currpcb->fdarray[fd].inode = currdentry->inode;
    currpcb->fdarray[fd].present = 1;
    currpcb->fdarray[fd].type = currdentry->ftype;


    /* allocate an unused file descriptor iff filename is not already present */

    /* set up any data necessary to handle the given type of file */

    /* if named file does not exist OR if no descriptor are free, then return -1 */
    return 0;
}

// Close: System Call Number 6
/*
int close(int32_t fd)
Description:
Inputs:
Outputs:
*/
int32_t close(int32_t fd)
{
    if((fd < 2) || (fd > MAX_FD_VAL)) return -1;    // cannot be stdin or stdout

    pcb_t * currpcb; 
    currpcb = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1));

    currpcb->(fdarray[fd].fileop)->open = 0;
    currpcb->(fdarray[fd].fileop)->read = 0;
    currpcb->(fdarray[fd].fileop)->write = 0;
    currpcb->(fdarray[fd].fileop)->close = 0;

    currpcb->fdarray[fd].inode = 0;
    currpcb->fdarray[fd].filepos = 0;
    currpcb->fdarray[fd].present = 0;
    currpcb->fdarray[fd].type = -1;
    //f2,f3 reserved (not used for now)
    currpcb->fdarray[fd].f2 = -1;
    currpcb->fdarray[fd].f3 = -1;

    return 0;
}

// Getargs: System Call Number 7
/*
int getargs(uint8_t* buf, int32_t nbytes)
Description:
Inputs:
Outputs:
*/
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    return 0;
}

// Vidmap: System Call Number 8
/*
int vidmap(uint8_t** screen_start)
Description:
Inputs:
Outputs:
*/
int32_t vidmap(uint8_t **screen_start)
{
    return 0;
}

// Set_handler: System Call Number 9
/*
int set_handler(int32_t signum, void* handler_address)
Description:
Inputs:
Outputs:
*/
int32_t set_handler(int32_t signum, void *handler_address)
{
    return 0;
}

// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/
int32_t sigreturn(void)
{
    return 0;
}



