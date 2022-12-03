#include "terminal.h"
#include "keyboard.h"
#include "lib.h"
#include "syscall.h"
#include "i8259.h"
#include "filesys.h"
#include "paging.h"

/*
int32_t terminal_open(const uint8_t* filename)
Description: Terminal's open function
Inputs: const uint8_t* filename = name of file to be opened
Outputs: returns int32_t = 0 on success
*/
//extern global_pcb;
extern int terminalrun;



int32_t terminal_open(const uint8_t *filename,int32_t fd){
    if(filename == NULL) return -1;
    return 0;
}

/*
int32_t terminal_close(int32_t fd)
Description: Terminal's close function
Inputs: int32_t fd = fild descriptor to close
Outputs: returns int32_t = 0 on success
*/
int32_t terminal_close(int32_t fd){
    if(fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    disable_cursor();
    return 0;
}

/*
int32_t terminal_read(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Terminal's read function. Reads from keyboardbuffer into the buffer passed in as buf
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = buffer to be written to
        int32_t nbytes = number of bytes to read from keyboardbuffer
Outputs: returns int32_t = number of bytes read
*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    uint8_t* buft = (uint8_t*)buf;
    if(buft == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) 
        return -1;
    int i;
    int count = 127;
    int j;
    if(nbytes > KEYBOARD_BUFFER_MAX_SIZE){   // handles overflow by just chopping off extra bytes
        nbytes = KEYBOARD_BUFFER_MAX_SIZE;
    }
    /* Will loop through infinitely waiting for an enter key from user to return with their input */
    while(1){
        if((currTerminal == terminalrun)&&(enterflag)){
            int nullflag;
            nullflag = 0;
            /* Will copy keyboardbuffer to buf */
            for(i=0; i < nbytes; i++){
                if(keyboardbuffer[i] == '\0' && !nullflag){
                    count = i;
                    nullflag = 1;
                }
                    //count = i;
                    buft[i] = keyboardbuffer[i];
            }
            /* Clears out keyboardbuffer */
            keyboardbuffersize = 0;
            while(keyboardbuffer[keyboardbuffersize] != '\n'){
                keyboardbuffersize++;
            }
            keyboardbuffersize++;
            for(j=0; j<KEYBOARD_BUFFER_MAX_SIZE; j++){
                keyboardbuffer[j] = '\0';   // clear our buffer to prevent an infinite loop of read/write on the same line 
            }
            terminalArray[currTerminal].currkey = 0;
            //charcount = 0;
            enterflag = 0;
            return count;
        }

    //return 0;
    }
}

/*
int32_t terminal_write(int32_t fd, uint8_t* buf, int32_t nbytes)
Description: Terminal's read function. Reads from keyboardbuffer into the buffer passed in as buf
Inputs: int32_t fd     = unused for now
        uint8_t* buf   = buffer to be read from 
        int32_t nbytes = number of bytes to write to screen from buf
Outputs: returns int32_t = 0 on success
*/
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    uint8_t* buft = (uint8_t*)buf;
    if(buft == NULL || fd > MAX_FD_VAL || fd < MIN_FD_VAL) return -1;
    if(nbytes < 0) return -1;
    int i;
    int linecount;

    //set_screen_x(0);
    
    linecount = charcount/NUM_COLS;
    charcount = 0;
    //set_screen_y(get_screen_y()-linecount); // handles unknown number of lines fed as input
    // if(keyboardbuffersize != 0)
    // putc2('\n');
    for(i=0; i<nbytes; i++){
        if(buft[i] != '\0'){
            if(buft[i] == '\t'){ // tab is equivalent to four spaces
                putc2(' ');
                putc2(' ');
                putc2(' ');
                putc2(' '); 
            }
            else {
                putc2(buft[i]); 
            }
        }
    }
    keyboardbuffersize = 0;

    return 0;
}

int32_t terminal_switch(int32_t newTerminal){
    // pcb_t * prev_pcb;
    if(newTerminal > 2 || newTerminal < 0)
        return -1;      // CHECK

    if(newTerminal == currTerminal){
            return 0;
    }
    //enable_irq(0);
    //cli();
    // terminalArray[currTerminal].cursor_x = get_screen_x();
    // terminalArray[currTerminal].cursor_y = get_screen_y();
        /* unmap current to itself */
    map_table((VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal)) >> PAGE_SHIFT  , (VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal))   );
    /* First copy vid mem to the actual terminal location */
    memcpy((uint8_t *)((VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal)) ), (uint8_t *)(VIDEO  ), FOUR_KILO_BYTE);

    /* Copy from the current terminal's keyboard buffer into the stored buffer */

    memcpy((uint8_t *)(terminalArray[currTerminal].terminalbuffer), (uint8_t *)(keyboardbuffer), KEYBOARD_BUFFER_MAX_SIZE);
    /* Copy from the stored buffer of the new terminal into the current terminal's keyboard buffer */
    memcpy((uint8_t *)(keyboardbuffer), (uint8_t *)(terminalArray[newTerminal].terminalbuffer), KEYBOARD_BUFFER_MAX_SIZE);

    /*newTerminal should map to vid mem*/
    /* Then copy from the new terminal location into vid mem */
    memcpy((uint8_t *)(VIDEO), (uint8_t *)(VIDEO_T1 + FOUR_KILO_BYTE * (newTerminal))  , FOUR_KILO_BYTE);
    map_table( (VIDEO_T1 + FOUR_KILO_BYTE * (newTerminal)) >> PAGE_SHIFT  , VIDEO  ); //???
    /* Update the current terminal's cursor */
    update_cursor(terminalArray[newTerminal].cursor_x, terminalArray[newTerminal].cursor_y);
    set_screen_x(terminalArray[newTerminal].cursor_x);
    set_screen_y(terminalArray[newTerminal].cursor_y);
    update_cursor(terminalArray[newTerminal].cursor_x, terminalArray[newTerminal].cursor_y);
    currTerminal = newTerminal;
    // prev_pcb = globalpcb;
    // register uint32_t saved_ebp asm("ebp");
    // register uint32_t saved_esp asm("esp");
    // prev_pcb->saved_esp = (void *)saved_esp;
    // prev_pcb->saved_ebp = (void *) saved_ebp;
    // globalpcb = terminalArray[newTerminal].cur_PCB;
    // if(terminalArray[newTerminal].cur_PCB == NULL){
    //     currpid++;
    //     execute((const uint8_t *)("shell"));
    // }else{
    //     /* Switch execution to current terminal's user program */
    //     uint32_t physaddr = (PDE_PROCESS_START + terminalArray[currTerminal].currprocessid) * FOUR_MB;
    //     map_helper(PDE_VIRTUAL_MEM, physaddr);
    //     tss.esp0 = EIGHT_MEGA_BYTE - EIGHT_KILO_BYTE * globalpcb->pid;
    //     /* (b) Set TSS for parent. ksp = kernel stack pointer */
    //     uint32_t args_esp = globalpcb->saved_esp;
    //     uint32_t args_ebp = globalpcb->saved_ebp;
    //     asm volatile(
    //         /* set esp, ebp as esp ebp args */
    //         "   movl %0, %%esp \n"
    //         "   movl %1, %%ebp \n"
    //         :
    //         : "r"(args_esp), "r"(args_ebp) // input
    //         : "cc"                         // ?
    //     );
    // }
    // sti();
    // if (!term_2_flag) {
    //     term_2_flag = 1;
    //     execute((const uint8_t *)("shell"));    //pid 0
    // }
    // if (!term_3_flag) {
    //     term_3_flag = 1;
    //     execute((const uint8_t *)("shell"));    //pid 0
    // }
    return 0;
}


void terminal_init(){
    int i; 
    currpid = 0;
    
    for(i = 0; i<MAX_TERMINALS; i++){
        processesid[i] = 1;    // SAYS PROCESS IS ACTIVE
        terminalArray[i].currRTC = -1;
        terminalArray[i].cur_PCB = NULL;
        // terminalArray[i].savedt_esp = ;
        // terminalArray[i].savedt_ebp = ;
        //terminalArray[i].terminalbuffer;

        terminalArray[i].cursor_x = 0;
        terminalArray[i].cursor_y = 0;
        // terminalArray[i].vidmemloc = (VIDEO_T1 + FOUR_KILO_BYTE *i);
        terminalArray[i].currprocessid = i;
        // currpid++;
        // if (i == 0)
        //     map_table(VIDEO_T1 >> PAGE_SHIFT, VIDEO);
        // else
        //     map_table((VIDEO_T1 + FOUR_KILO_BYTE *i) >> PAGE_SHIFT, (VIDEO_T1 + FOUR_KILO_BYTE *i));
    }
    terminalArray[0].vidmemloc = (uint32_t)(VIDEO_T1);
    terminalArray[1].vidmemloc = (uint32_t)(VIDEO_T2);
    terminalArray[2].vidmemloc = (uint32_t)(VIDEO_T3);
    currpid = -1;
    currTerminal = 0;
    // runningterminal = 0;
   // execute((const uint8_t *)("shell"));    //pid 0

}

