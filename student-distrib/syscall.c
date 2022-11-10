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

/* System Call Functions */

/*
int execute(const uint8_t* command)
Inputs:         command - gives the command + args from the terminal
Outputs:        integer - whether it happened successfully (0) or not (-1).
Description:    It helps us execute the functions on request by the user. 

Things done in execute:

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
int32_t execute(const uint8_t *command)
{

    /* Checks if we have max number of processes: */
    if (currpid >= MAX_NUM_PROCESSES) {
        puts2("Too many processes called! (>6)\n", ERRMSG);
        return 0;
    }

    /*  1. Extract name and args - check whether executable  */

    int cmd_len = strlen((const int8_t *)(command));
    uint8_t filename[MAX_FILENAME_LEN];
    uint8_t finalarg[MAX_ARG_LEN];
    uint8_t buffer[4];
    int i = 0;
    int filechar = 0;
    int finalchar = 0;


    /* Initialize the name, args as NULL */
    for (i = 0; i < MAX_FILENAME_LEN; i++) {
        filename[i] = '\0';
    }
    
    for (i = 0; i < MAX_ARG_LEN; i++) {
        finalarg[i] = '\0';
    }
    
    i = 0;

    int start = 0;
    /* Parse the args / command name */
    for (; i < cmd_len; i++) {
        if (start == 0) {
            if (command[i] == ' ')
                continue;
            start = 1;
            filename[filechar++] = command[i];
        } 
        else {
            if (command[i] == ' ')
                break;
            filename[filechar++] = command[i];
        }
    } 
    start = 0;
    for (; i < cmd_len; i++) {
        if (start == 0) {
            if (command[i] == ' ')
                continue;
            start = 1;
            finalarg[finalchar++] = command[i];
        } 
        else {
            finalarg[finalchar++] = command[i];
        }
    }



    /* 2. Search file system for the name of the file given: store it in the currdentry. */
    dentry_t currdentry;
    
    if (strlen((int8_t *)(filename)) == 0 || read_dentry_by_name(filename, (dentry_t*)(&currdentry)) < 0 )
        return -1;                              // Invalid filename / could not find file


    /* 3. The first 4 bytes of the file represent a “magic number” that identifies the file as an executable. These
    bytes are, respectively, 0: 0x7f; 1: 0x45; 2: 0x4c; 3: 0x46. If the magic number is not present, the execute system
    call should fail.
    */

    int exec = 0; // Check if it is an executable
    
    /* Store the 4 magic numbers into the buffer to be checked */
    if (read_data(currdentry.inode, 0, (uint8_t *)(buffer), BYTES_TO_COPY) < 0 )
        return -1;                              // Could not successfully extract the numbers
    
    /* Check the 4 bytes 0,1,2,3 in the buffer about whether those are magic numbers */
    if (buffer[0] == MAGIC_0 && buffer[1] == MAGIC_1 && buffer[2] == MAGIC_2 && buffer[3] == MAGIC_3) // magic numbers for executables
        exec = 1;
    if (!exec) 
        return -1;                              // it is not executable



    /* 4. Paging: Use PID to decide 8 + (PID*4MB)
          Then copy the whole file into that virtual address 128MB location.
    */
    
    // Future checkpoints: make an array of structs. Right now, just use a single pid thing. 
    currpid++;                                  // New process is active.
    
    uint32_t physaddr = (PDE_PROCESS_START + currpid) * FOUR_MB;
    page_directory[PDE_VIRTUAL_MEM].ps = 1;             // make it a 4 mb page
    page_directory[PDE_VIRTUAL_MEM].pt_baddr = physaddr >> PAGE_SHIFT;
    page_directory[PDE_VIRTUAL_MEM].g = 0;              // Want page to be flushed when tlb is flushed
    page_directory[PDE_VIRTUAL_MEM].pcd = 1;            // in desc.pdf
    page_directory[PDE_VIRTUAL_MEM].us = 1;             // must be 1 for all user-level pages and mem ranges
    page_directory[PDE_VIRTUAL_MEM].p = 1;              // the page is present.
    /* Flush the TLB */
    loadPageDir(page_directory); // flush TLB //? check with os dev maybe other stuff for flushing 

    /* Now we start copying into 4MB User pages */
    uint8_t *addrptr = (uint8_t *)(VIRT_ADDR); // PASSED INTO READ DATA AS BUFFER
    uint32_t currdentryinodenum = currdentry.inode;
    uint32_t currdentryinodelen = ((inode_t *)(inodeptr + currdentryinodenum))->length;
    
    /* Uses read_data to copy information into the user page. */
    if (read_data(currdentryinodenum, 0, addrptr, currdentryinodelen) < 0 )
        return -1;

    cli();


    /* 5. Create new PCB for the current newly created process. */
    
    int curraddr = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid + 1));
    pcb_t * currpcb; 
    currpcb =  (pcb_t *)(curraddr);
    
    currpcb->pid = currpid;
    if (currpid == 0)
        currpcb-> parent_id = -1; // check what parent of shell should be 
    else
        currpcb-> parent_id = currpid - 1;

    uint32_t save_esp = 0;
    uint32_t save_ebp = 0;

    asm volatile
    (
        "movl %%ebp, %0; \n"
        "movl %%esp, %1; \n"
        :"=g"(save_ebp), "=g"(save_esp)
        :
    );

    currpcb->saved_esp = save_esp;
    currpcb->saved_ebp = save_ebp;
    
    /*  Set up the FD (1, 2 for terminal, rest empty) */
    for(i = 0; i<MAX_FD_LEN; i++) {

        (currpcb->fdarray[i]).fileop.open = 0;
        (currpcb->fdarray[i]).fileop.read = 0;
        (currpcb->fdarray[i]).fileop.write = 0;
        (currpcb->fdarray[i]).fileop.close = 0;

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
    (currpcb->fdarray[0]).fileop.open = terminal_open;
    (currpcb->fdarray[0]).fileop.read = terminal_read; 
    (currpcb->fdarray[0]).fileop.write = NULL;
    (currpcb->fdarray[0]).fileop.close = terminal_close; 
    (currpcb->fdarray[0]).present = 1;

    // std out
    (currpcb->fdarray[1]).fileop.open = terminal_open;
    (currpcb->fdarray[1]).fileop.read = NULL;
    (currpcb->fdarray[1]).fileop.write = terminal_write;
    (currpcb->fdarray[1]).fileop.close = terminal_close;
    (currpcb->fdarray[1]).present = 1;
    

    globalpcb = currpcb;
    
    strcpy(globalpcb->argbuffer, finalarg);

    /* 6. Set up context switch ( kernel stack base into esp of tss ) */
 
    uint32_t currksp = (uint32_t)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * currpid) - FOUR_BYTE_OFFSET);
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
    uint32_t stack_eip = 0; // buffer[3],buffer[2],buffer[1],buffer[0]
    /* Reversing 4 bytes of the buffer (bytes 24-27 of the file) due to little endianness*/
    stack_eip = (stack_eip | buffer[3]);
    stack_eip = stack_eip << BUFFER_SHIFT;
    stack_eip = (stack_eip | buffer[2]);
    stack_eip = stack_eip << BUFFER_SHIFT;
    stack_eip = (stack_eip | buffer[1]);
    stack_eip = stack_eip << BUFFER_SHIFT;
    stack_eip = (stack_eip | buffer[0]);


    uint32_t stack_esp = PROG_START - FOUR_BYTE_OFFSET;
    int usrDS = USER_DS;
    int usrCS = USER_CS;
    sti();

    /* Start doing IRET */    
    /* 
    Stuff to push to stack (to convert to usermode) in order:
    USER_DS
    USER_ESP
    FLAGS | 200
    USER_CS
    PROG_EIP
    The 4 bytes we just extracted was the EIP - we need to save that because it is what we push in IRET.
    The ESP is basically the 132 MB location - 4 Bytes so we do not exceed the virtual stack location.
    */
   /* Oring to enable interrupts - 0x0200 with the flags, currently stored in eax */
    
    asm volatile("pushl %0;"
        "pushl %1; \n"
        "pushfl; \n"
        "popl %%eax; \n"
        "orl $0x0200, %%eax; \n"
        "pushl %%eax; \n"
        "pushl %2; \n"
        "pushl %3; \n"
        "iret; \n"
        :                            
        : "g"(usrDS), "g"(stack_esp), "g"(usrCS), "g"(stack_eip)
        :"%eax", "memory", "cc"
        );
        
    return 0;
}


/*
int halt(uint8_t status)

Inputs:             status - tells us which program called halt.
Outputs:            whether the halt call was successful (0) or not (-1)
Description:        The halt system call terminates a process, returning the specified value to its parent process. The system call handler
                    itself is responsible for expanding the 8-bit argument from BL into the 32-bit return value to the parent program’s
                    execute system call. Be careful not to return all 32 bits from EBX. This call should never return to the caller.
    
    1. Restore Parent data (stored in the PCB)
    2. Restore Parent paging
    3. Close the relevant FDs
    4. Jump to the execute's return

    1. Setup return value (check if exception // check if program is finished)
    2. close all processes
    3. set currently active process to non active
    4. check if it is the main shell (restart if yes)
    5. not main shell handler
    6. halt return (assembly)

*/
int32_t halt(uint8_t status)
{
   
/* 1. Set up return value:
    - expand the 8-bit arg from BL into the 32-bit return value to the parent program
    - 8-bit 'status' arguement = the (one of 256) interupt handler that called the 'halt' */

    uint32_t status_ret_val = 0x0000;

    if (status == HALT_CODE) {
        status_ret_val = EXCEPTION_CALLED;
    }


    int parent_pcbaddr;   
    pcb_t* currpcb = (pcb_t *)globalpcb;
    pcb_t* parentpcb;

    /* 2. Close all processes
    - Close all processes except the parent execute system call 
    - after 2.a, all currpcb == globalpcb AND all parentpcb == globalpcb - 1 */
    
    /* (a) check if the current process's parent is the shell program*/
    if (currpid == 0 || currpcb->parent_id == -1){
        currpid--;
        execute((const uint8_t *)"shell");
    }
    
    parent_pcbaddr = EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid));
    parentpcb = (pcb_t *)(parent_pcbaddr); 

    /* 3. set the current pcb->active bit to 0 (non active)*/
    currpcb->active = 0;

    /* Close all things in fd table of the currpcb (or globalpcb)*/
    int i; int close_result;
    for(i=MIN_FD_VAL_STD; i < MAX_FD_LEN; i++){
        if(globalpcb->fdarray[i].present == 1) {
            close_result = close(i);
            if (close_result < 0)
                return -1; // change!! error check
        }
    }


    /* 5. not main shell handler
    - Get parent process
    - Set the TSS for parent
    - Unmap pages for current process
    - Map pages for parent process
    - Set parent's process as active
    - Call halt return (assembly)
    */

    /* (a) Get parent Process */
    /* parentpcb was set at the begining: Step 2*/


    
    /* (c) Unmap pages for current process */
    /* currpid = static int global variable */
   
    page_directory[PDE_VIRTUAL_MEM].p           = 0;
    currpid--;

    /* (d) Map pages for parent process */
    /* currpid was decremented, now currpid is set to the parent (currpid - 1)*/
    int parentpid = currpid;
    uint32_t parent_physaddr = (PDE_PROCESS_START + parentpid) * FOUR_MB;
    page_directory[PDE_VIRTUAL_MEM].ps = 1; // make it a 4 mb page
    page_directory[PDE_VIRTUAL_MEM].pt_baddr = parent_physaddr >> PAGE_SHIFT;
    page_directory[PDE_VIRTUAL_MEM].g = 0;              // Want page to be flushed when tlb is flushed
    page_directory[PDE_VIRTUAL_MEM].pcd = 1;            // in desc.pdf
    page_directory[PDE_VIRTUAL_MEM].us = 1;             // must be 1 for all user-level pages and mem ranges
    page_directory[PDE_VIRTUAL_MEM].p = 1;         
    /* Flush the TLB */
    loadPageDir(page_directory);

    /* (e) Set Parents Process as active */
    parentpcb->active = 1;

    /* (b) Set TSS for parent. ksp = kernel stack pointer */
    uint32_t args_esp = currpcb->saved_esp;
    uint32_t args_ebp = currpcb->saved_ebp;
    uint32_t parentksp = (uint32_t)(EIGHT_MEGA_BYTE - (EIGHT_KILO_BYTE * (currpid) ) - FOUR_BYTE_OFFSET);
    tss.ss0 = KERNEL_DS;
    tss.esp0 = parentksp;
    globalpcb = parentpcb;

    /* 6. halt return (assembly)
    take in esp, ebp, retval
    set esp, ebp as esp ebp args
    set eax regs as ret val
    */
    asm volatile
    (

        /* set esp, ebp as esp ebp args */
        "   movl %0, %%esp \n"
        "   movl %1, %%ebp \n"
        /* set eax regs as ret val */
        "   movl %2, %%eax \n"
        "   leave;          \n"
        "   ret;            \n"
        :
        : "r"(args_esp), "r"(args_ebp), "r"(status_ret_val)             // input
        : "cc"
    );

    return 0;
}

/*
int read(int32_t fd, void* buf, int32_t nybytes)
Inputs:             fd: the file descriptor
                    buf: where we need to copy into
                    nbytes: number of bytes to be copied
Outputs:            whether call was successful (0) or not (-1)
Description:        reads data from keyboard , file, device(RTC), or directory
*/
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    sti();

    /* sanity check: initial file position at eof or beyonf end of curr file */
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL || nbytes < 0) {return -1;}

    /* use a jmptable referenced by the tasks files array...
    which calls a generic handler for the specific file type's specific read function */

    if(fd == 1) return -1;                                  // This is terminal_write

    if((fd == 0))
        return terminal_read(fd, buf, nbytes);

    if(globalpcb->fdarray[fd].present == 0) 
    return -1;                                              // FD is absent 


    int rval = ((globalpcb->fdarray[fd]).fileop.read)(fd, buf, nbytes);
    if (rval < 0)
        return -1;
    return rval;
}


/*
int write(int32_t fd, void* buf, int32_t nbytes)
Inputs:             fd: the file descriptor
                    buf: stuff to copy
                    nbytes: number of bytes to be copied
Outputs:            whether call was successful (0) or not (-1)
Description:        writes data to either the terminal or to a device (RTC), depending on the fd
*/
int32_t write(int32_t fd, void *buf, int32_t nbytes)
{
    
    /* sanity check: if device(RTC), else we write to terminal */
    if(buf == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL || nbytes < 0) 
        return -1;

    /* if RTC: syscall should always accept a 4-byte int specifyinng the interrupt rate in Hz (should set the rate of periodic interuppts accordingly) */
    if(fd == 0) 
        return -1;                                          // This is terminal_read

    if((fd == 1))
        return terminal_write(fd, buf, nbytes);

    if(globalpcb->fdarray[fd].present == 0) 
        return -1;                                          // FD is absent

    int rval = ((globalpcb->fdarray[fd]).fileop.write)(fd, buf, nbytes); // not sure how to call function
    if (rval < 0)
        return -1;
    
    return nbytes;
}


/*
int open(const uint8_t* filename)

Inputs:                 filename: the file to be opened
Outputs:                whether call was successful (0) or not (-1)
Description:            Opens the respective file (rtc, terminal, file or directory)
*/
int32_t open(const uint8_t *filename)
{
    int i;
    /* find the dir entry corresponding to the named file */
    if(filename == NULL) return -1;

    int dentry_name_return;
    dentry_t currdentry;


    dentry_name_return = read_dentry_by_name(filename, (dentry_t *)(&currdentry));
    if(dentry_name_return == -1) 
    return -1;                                  // check if file exists

    /* Traverses through the file descriptor array for the  free fd entry. */
    int fd = -1;
     for(i = START_FD_VAL; i<MAX_FD_LEN; i++){
        if(!(globalpcb->fdarray[i]).present){
            fd = i;
            break;
        } 
    }

    if(fd == -1) 
    return -1;                                  // fd array is full, failed to open file

    /* allocate an unused file descriptor iff filename is not already present */
    /* RTC */
    if(currdentry.ftype == TYPE_RTC){
        (globalpcb->fdarray[fd]).fileop.open = open_rtc;
        (globalpcb->fdarray[fd]).fileop.read = read_rtc;
        (globalpcb->fdarray[fd]).fileop.write = write_rtc;
        (globalpcb->fdarray[fd]).fileop.close = close_rtc;
        (globalpcb->fdarray[fd]).filepos = -1;
    }
    /* DIRECTORY */
    else if(currdentry.ftype == TYPE_DIR){
        (globalpcb->fdarray[fd]).fileop.open = open_dir;
        (globalpcb->fdarray[fd]).fileop.read = read_dir;
        (globalpcb->fdarray[fd]).fileop.write = write_dir;
        (globalpcb->fdarray[fd]).fileop.close = close_dir;
        (globalpcb->fdarray[fd]).filepos = 0;
    }
    /* NORMAL FILE */
    else if(currdentry.ftype == TYPE_FILE){
        (globalpcb->fdarray[fd]).fileop.open = open_file;
        (globalpcb->fdarray[fd]).fileop.read = read_file;
        (globalpcb->fdarray[fd]).fileop.write = write_file;
        (globalpcb->fdarray[fd]).fileop.close = close_file;
        (globalpcb->fdarray[fd]).filepos = 0;
    }
    (globalpcb->fdarray[fd]).inode = currdentry.inode;
    (globalpcb->fdarray[fd]).present = 1;
    (globalpcb->fdarray[fd]).type = currdentry.ftype;
    
    int rval =  (globalpcb->fdarray[fd]).fileop.open(filename);
    
    /* if named file does not exist OR if no descriptor are free, then return -1 */
    if (rval < 0)
        return -1;

    return fd;
}

/*
int close(int32_t fd)
Inputs:                 fd: the file to be closed
Outputs:                whether call was successful (0) or not (-1)
Description:            Closes the respective file (rtc, terminal, file or directory)
*/
int32_t close(int32_t fd)
{
    if((fd < MIN_FD_VAL_STD) || (fd > MAX_FD_VAL)) 
    return -1;                                          // cannot be stdin or stdout

    if ((globalpcb->fdarray[fd]).present == 0)
        return -1;

    int rval = ((globalpcb->fdarray[fd]).fileop.close)(fd); // try closing the file
    if (rval < 0)
        return -1;                                          // could not close the file
        
    // Otherwise you can safely close it.
    (globalpcb->fdarray[fd]).fileop.open = 0;
    (globalpcb->fdarray[fd]).fileop.read = 0;
    (globalpcb->fdarray[fd]).fileop.write = 0;
    (globalpcb->fdarray[fd]).fileop.close = 0;

    (globalpcb->fdarray[fd]).inode = 0;
    (globalpcb->fdarray[fd]).filepos = 0;
    (globalpcb->fdarray[fd]).present = 0;
    (globalpcb->fdarray[fd]).type = -1;

    //f2,f3 reserved (not used for now)
    (globalpcb->fdarray[fd]).f2 = -1;
    (globalpcb->fdarray[fd]).f3 = -1;

    return 0;
}

/*
int getargs(uint8_t* buf, int32_t nbytes)
Description:
Inputs:
Outputs:
*/
int32_t getargs(uint8_t *buf, int32_t nbytes)
{

    // int cmd_len = strlen((const int8_t *)(buf));
    // uint8_t arg1[MAX_FILENAME_LEN];
    // uint8_t arg2[MAX_FILENAME_LEN];
    // uint8_t arg3[MAX_FILENAME_LEN];
    // int i = 0;
    // int arg1char = 0;
    // int arg2char = 0;
    // int arg3char = 0;
    // for (i = 0; i < MAX_FILENAME_LEN; i++) {
    //     arg1[i] = '\0';
    //     arg2[i] = '\0';
    //     arg3[i] = '\0';
    // }
    // i = 0;
    // int start = 0;
    // for (; i < cmd_len; i++) {
    //     if (start == 0) {
    //         if (buf[i] == ' ')
    //             continue;
    //         start = 1;
    //         arg1[arg1char++] = buf[i];
    //     } 
    //     else {
    //         if (buf[i] == ' ')
    //             break;
    //         arg1[arg1char++] = buf[i];
    //     }
    // } 

    // start = 0;
    // for (; i < cmd_len; i++) {
    //     if (start == 0) {
    //         if (buf[i] == ' ')
    //             continue;
    //         start = 1;
    //         arg2[arg2char++] = buf[i];
    //     } 
    //     else {
    //         if (buf[i] == ' ')
    //             break;
    //         arg2[arg2char++] = buf[i];
    //     }
    // } 
    // start = 0;
    // for (; i < cmd_len; i++) {
    //     if (start == 0) {
    //         if (buf[i] == ' ')
    //             continue;
    //         start = 1;
    //         arg3[arg3char++] = buf[i];
    //     } 
    //     else {
    //         if (buf[i] == ' ')
    //             break;
    //         arg3[arg3char++] = buf[i];
    //     }
    // } 

    if(strlen((uint8_t *)globalpcb->argbuffer) == 0) return -1;

    strncpy((uint8_t *)buf, (uint8_t *)(globalpcb->argbuffer), nbytes);

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






