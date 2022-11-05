#include "syscall.h"
#include "x86_desc.h"
#include "IDT.h"
#include "lib.h"
#include "linkageheader.h"
#include "filesys.h"
#include "paging.h"

#include "terminal.h"

/* Variables for the different system calls */

static int currpid = -1;
pcb_t *globalpcb;
extern page_dir_entry page_directory[1024] __attribute__((aligned(SIZE_OF_PG)));
extern page_table_entry page_table[1024] __attribute__((aligned(SIZE_OF_PG)));

/* System Call Functions */

// Halt: System Call Number 1
/*
int halt(uint8_t status)
Description:        The halt system call terminates a process, returning the specified value to its parent process. The system call handler
                    itself is responsible for expanding the 8-bit argument from BL into the 32-bit return value to the parent program’s
                    execute system call. Be careful not to return all 32 bits from EBX. This call should never return to the caller.

Inputs: status
Outputs:
*/
int32_t halt(uint8_t status)
{
   
    /* Order of stuff to write:

    1. Restore Parent data (stored in the PCB)
    2. Restore Parent paging
    3. Close the relevant FDs
    4. Jump to the execute's return

    // okan:
    1. Setup return value (check if exception // check if program is finished)
    2. close all processes
    3. set currently active process to non active
    4. check if it is the main shell (restart if yes)
    5. not main shell handler
    6. halt return (assembly)
    
    */

    /* 1. Set up return value: */
    

    /* 2. Close all processes */

    // int shellflag = 0;
    // if (globalpcb->parent_id == -1)
    //     shellflag = 1;

    // pcb_t * parentpcb; 
    // if (shellflag)
    //     parentpcb =  (uint32_t *)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (1)));
    



   /* 5. not main shell handler
   - Get parent process
   - Set the TSS for parent
   - Unmap pags for current process
   - Map pages for parent process
   - Set parent's process as active
   - Call halt return (assembly)
   */



    /* 6. halt return (assembly)
    take in esp, ebp, retval
    set esp, ebp as esp ebp args
    set eax regs as ret val
    */



    //cli()

    //sti()
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

    int cmd_len = strlen((const int8_t *)(command));
    uint8_t filename[MAX_FILENAME_LEN];
    uint32_t arg0[MAX_ARG_LEN];
    // uint32_t arg1[32];
    // uint32_t arg2[32];
    uint32_t buffer[4];
    uint32_t offset = 0;
    int i = 0, argflag = -1;
    int filenamechar = 0, arg0char = 0;
    // int arg1char = 0, arg2char = 0;

    /* Initialize the name, args as null */
    for (i = 0; i < MAX_ARG_LEN; i++) {      // change
        if (i < MAX_FILENAME_LEN)
            filename[i] = '\0';
        arg0[i] = '\0';
        // arg1[i] = '\0';
        // arg2[i] = '\0';
    }

    /* Parse the args / command name */
    for (i = 0; i < cmd_len; i++) {
        if (command[i] == ' ') {
            argflag++;
            continue;
        }
        switch (argflag) {

            case -1:
            if (filenamechar < MAX_FILENAME_LEN)
                filename[filenamechar++] = command[i];
            else
                return -1;
            break;

            case 0:
            if (arg0char < MAX_ARG_LEN)// change
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
    if (strlen((int8_t *)(filename)) == 0 || read_dentry_by_name(filename, (dentry_t*)(&currdentry)) < 0 )
        return -1;              // Invalid filename / could not find file
    


    /* 3. Extract all information about the file - if it is executable                          // read data 

    The layout of executable files in the file system is simple: the entire file stored in the file system is the image of the
    program to be executed. In this file, a header that occupies the first 40 bytes gives information for loading and starting
    the program. The first 4 bytes of the file represent a “magic number” that identifies the file as an executable. These
    bytes are, respectively, 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46. If the magic number is not present, the execute system
    call should fail.
    */

    int exec = 0; // not an executable
    
    // Prototype: int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) 
    /* Store the 4 magic numbers into the buffer to be checked */
    if (read_data(currdentry.inode, 0, (uint8_t *)(buffer), BYTES_TO_COPY) < 0 )
        return -1;              // Invalid filename / could not find file
    
    /* Check the 4 bytes 0,1,2,3 in the buffer about whether those are magic numbers */
    if (buffer[0] == MAGIC_0 && buffer[1] == MAGIC_1 && buffer[2] == MAGIC_2 && buffer[3] == MAGIC_3) // magic numbers for executables
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
    
    loadPageDir(page_directory); // flush TLB //? check with os dev maybe other stuff for flushing 

    /* Now we start copying into 4MB User pages */
    uint8_t *addrptr = (uint8_t *)(VIRT_ADDR); // PASSED INTO READ DATA AS BUFFER
    uint32_t currdentryinodenum = currdentry.inode;
    uint32_t currdentryinodelen = ((inode_t *)(inodeptr + currdentryinodenum))->length;
    // Prototype: int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    // Check offset and buffer size, numbytes
    
    if (read_data(currdentryinodenum, 0, addrptr, currdentryinodelen) < 0 )
        return -1;



    /* 5. Set up PCB stuff too - create new PCB */
    
    pcb_t * currpcb; 
    currpcb =  (pcb_t *)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1)));
    
    
    currpcb->pid = currpid;
    if (currpid == 0)
        currpcb-> parent_id = -1; // check what parent of shell should be 
    else
        currpcb-> parent_id = currpid - 1;

    // register uint32_t save_esp = asm("esp");   // CHECK  
    // register uint32_t save_ebp = asm("ebp"); 
    // uint32_t save_esp asm("s_esp") = 0;   // CHECK  
    uint32_t save_esp = 0;
    // uint32_t save_ebp asm("s_ebp") = 0;     
    uint32_t save_ebp = 0;
    asm volatile
    (
        "movl %%ebp, %0"
        "movl %%esp, %1"
        :"=g"(save_ebp), "=g"(save_esp)
        :
    );

    
    // currpcb->saved_esp = save_esp;
    // currpcb->saved_ebp = save_ebp;

    currpcb->saved_esp = save_esp;
    currpcb->saved_ebp = save_ebp;
    
    /*  Set up the FD (1, 2 for terminal, rest empty) */

    for(i = 0; i<MAX_FD_LEN; i++){//maybe init the entry 0 and 1 for stdin and stdout
        (currpcb->fdarray[i]).fileop->open = 0; //should we set them to the fail funcs?
        (currpcb->fdarray[i]).fileop->read = 0;
        (currpcb->fdarray[i]).fileop->write = 0;
        (currpcb->fdarray[i]).fileop->close = 0;

        (currpcb->fdarray[i]).inode = -1;
        (currpcb->fdarray[i]).filepos = 0;
        (currpcb->fdarray[i]).present = 0;
        (currpcb->fdarray[i]).type = -1;
        //f2,f3 reserved (not used for now)
        (currpcb->fdarray[i]).f2 = -1;
        (currpcb->fdarray[i]).f3 = -1;
    }
    currpcb->active = 1;

    // std in
    (currpcb->fdarray[0]).fileop->open = terminal_open;
    (currpcb->fdarray[0]).fileop->read = terminal_read; 
    (currpcb->fdarray[0]).fileop->write = NULL;
    (currpcb->fdarray[0]).fileop->close = terminal_close; 
    (currpcb->fdarray[0]).present = 1;

    // std out
    (currpcb->fdarray[1]).fileop->open = terminal_open;
    (currpcb->fdarray[1]).fileop->read = NULL;
    (currpcb->fdarray[1]).fileop->write = terminal_write;
    (currpcb->fdarray[1]).fileop->close = terminal_close;
    (currpcb->fdarray[1]).present = 1;

    globalpcb = currpcb;

    /* 6. Set up context switch ( kernel stack base into esp of tss ) */

    uint32_t currksp = (uint32_t)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * currpid));
    tss.ss0 = KERNEL_DS;
    tss.esp0 = currksp;

    /* 
     The other important bit of information that you need to execute programs is the entry point into the
    program, i.e., the virtual address of the first instruction that should be executed. This information is stored as a 4-byte
    unsigned integer in bytes 24-27 of the executable, and the value of it falls somewhere near 0x08048000 for all programs we have provided to you. When processing the execute system call, your code should make a note of the entry
    point, and then copy the entire file to memory starting at virtual address 0x08048000. It then must jump to the entry
    point of the program to begin execution.
    */
    if (read_data(currdentry.inode, VIRT_ADDR_INSTRUC, buffer, BYTES_TO_COPY) < 0 )
        return -1;              // could not read those 4 bytes.

    /* 
    Stuff to push to stack (to convert to usermode) in order:
    USER_DS
    USER_ESP
    // flag ???
    USER_CS
    PROG_EIP
    The 4 bytes we just extracted was the EIP - we need to save that because it is what we push in IRET.
    The ESP is basically the 132 MB location - 4 Bytes so we do not exceed the virtual stack location.
    */
    uint32_t stack_eip = buffer;
    uint32_t stack_esp = PROG_START - FOUR_BYTE_OFFSET;

    //!!! NEED TO ADD CLI AND STU SOMEWHERE HERE 
    int usrDS = USER_DS;
    int usrCS = USER_CS;

    /* Start doing IRET */
    // pop flags into eax, and the val in eax 0x0200, push that
    asm volatile("pushl %0;
        pushl %1;
        pushlfl;
        popl %%eax;
        orl $0x0200, %%eax;
        pushl %%eax;
        pushl %2;
        pushl %3;
        iret;"

        :                            
        : "=g"(usrDS), "=g"(stack_esp), "=g"(usrCS), "=g"(stack_eip)
        :"%eax"
        );


    return 0;
}

/* Heloer mapping function*/
 
// int32_t map()

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

    return ((globalpcb->fdarray[fd]).fileop->read)(fd, buf, nbytes); // not sure how to call function

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

    return ((globalpcb->fdarray[fd]).fileop->write)(fd, buf, nbytes); // not sure how to call function

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
    int i;
    /* find the dir entry corresponding to the named file */
    if(filename == NULL) return -1;

    int dentry_name_return;
    dentry_t *currdentry;
    dentry_name_return = read_dentry_by_name(filename, (dentry_t *)(&currdentry));
    if(dentry_name_return == -1) return -1; // check if file exists

    int fd = -1;
     for(i = START_FD_VAL; i<MAX_FD_LEN; i++){
        if(!(globalpcb->fdarray[i]).present){
            fd = i;
            break;
        } 
    }

    if(fd == -1) return -1; // fd array is full, failed to open file

   // if((currdentry->ftype < 0) || (currdentry->ftype > 2)) return -1;
   //? made the init the fail funcs so if below conditons not set them returns -1 when they are called
    
    if(currdentry->ftype == 0){    // rtc
        (globalpcb->fdarray[fd]).fileop->open = open_rtc;
        (globalpcb->fdarray[fd]).fileop->read = read_rtc;
        (globalpcb->fdarray[fd]).fileop->write = write_rtc;
        (globalpcb->fdarray[fd]).fileop->close = close_rtc;
        (globalpcb->fdarray[fd]).filepos = -1;                  // why start at -1??? why not 0?
    }
    else if(currdentry->ftype == 1){    // dir
        (globalpcb->fdarray[fd]).fileop->open = open_dir;
        (globalpcb->fdarray[fd]).fileop->read = read_dir;
        (globalpcb->fdarray[fd]).fileop->write = write_dir;
        (globalpcb->fdarray[fd]).fileop->close = close_dir;
        (globalpcb->fdarray[fd]).filepos = 0;
    }
    else if(currdentry->ftype == 2){    // normal file
        (globalpcb->fdarray[fd]).fileop->open = open_file;
        (globalpcb->fdarray[fd]).fileop->read = read_file;
        (globalpcb->fdarray[fd]).fileop->write = write_file;
        (globalpcb->fdarray[fd]).fileop->close = close_file;
        (globalpcb->fdarray[fd]).filepos = 0;
    }
    (globalpcb->fdarray[fd]).inode = currdentry->inode;
    (globalpcb->fdarray[fd]).present = 1;
    (globalpcb->fdarray[fd]).type = currdentry->ftype;
    
    int numb =  (globalpcb->fdarray[fd]).fileop->open(filename); // not sure how to call function
    
    if (numb < 0)
        return -1;

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

    if ((globalpcb->fdarray[fd]).present == 0)
        return -1;

    // Otherwise you can safely close it.
    (globalpcb->fdarray[fd]).fileop->open = 0;
    (globalpcb->fdarray[fd]).fileop->read = 0;
    (globalpcb->fdarray[fd]).fileop->write = 0;
    (globalpcb->fdarray[fd]).fileop->close = 0;

    (globalpcb->fdarray[fd]).inode = 0;
    (globalpcb->fdarray[fd]).filepos = 0;
    (globalpcb->fdarray[fd]).present = 0;
    (globalpcb->fdarray[fd]).type = -1;
    //f2,f3 reserved (not used for now)
    (globalpcb->fdarray[fd]).f2 = -1;
    (globalpcb->fdarray[fd]).f3 = -1;

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

/*-------------fail helper funcs--------------*/
// read_fail
/*
Description:
Inputs:
Outputs: returns -1
*/
int32_t read_fail(const uint8_t *filename){
    return -1;
}
// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/
int32_t write_fail(int32_t fd){
    return -1;
}
// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/ 
int32_t open_fail(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}
// Sigreturn: System Call Number 10
/*
int sigreturn(void)
Description:
Inputs:
Outputs:
*/
int32_t close_fail(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}  






