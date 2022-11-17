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
int32_t terminal_open(const uint8_t* filename){
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
    int count;
    int j;
    if(nbytes > KEYBOARD_BUFFER_MAX_SIZE){   // handles overflow by just chopping off extra bytes
        nbytes = KEYBOARD_BUFFER_MAX_SIZE;
    }
    /* Will loop through infinitely waiting for an enter key from user to return with their input */
    while(1){
        if(enterflag){
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
            currkey = 0;
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
    if(newTerminal > 2 || newTerminal < 0)
        return -1;      // CHECK

    if(newTerminal == currTerminal){
            return 0;
    }
    terminalArray[currTerminal].cursor_x = get_screen_x();
    terminalArray[currTerminal].cursor_y = get_screen_y();
    /* First copy vid mem to the actual terminal location */
    // memcpy((uint8_t *)((VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal)) >> PAGE_SHIFT), (uint8_t *)(VIDEO >> PAGE_SHIFT), FOUR_KILO_BYTE);
    memcpy((uint8_t *)((VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal)) ), (uint8_t *)(VIDEO  ), FOUR_KILO_BYTE);
    /* Then copy from the new terminal location into vid mem */
    memcpy(VIDEO  , (VIDEO_T1 + FOUR_KILO_BYTE * (newTerminal))  , FOUR_KILO_BYTE);
    /* Copy from the current terminal's keyboard buffer into the stored buffer */
    memcpy(terminalArray[currTerminal].terminalbuffer, keyboardbuffer, KEYBOARD_BUFFER_MAX_SIZE);
    /* Copy from the stored buffer of the new terminal into the current terminal's keyboard buffer */
    memcpy(keyboardbuffer, terminalArray[newTerminal].terminalbuffer, KEYBOARD_BUFFER_MAX_SIZE);
    /* unmap current to itself */
    map_table((VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal))  , (VIDEO_T1 + FOUR_KILO_BYTE * (currTerminal))   );
    /*newTerminal should map to vid mem*/
    map_table((VIDEO_T1 + FOUR_KILO_BYTE * (newTerminal))  , VIDEO  ); //???
    /* Update the current terminal's cursor */
    
    update_cursor(terminalArray[newTerminal].cursor_x, terminalArray[newTerminal].cursor_y);
    set_screen_x(terminalArray[newTerminal].cursor_x);
    set_screen_y(terminalArray[newTerminal].cursor_y);
    update_cursor(terminalArray[newTerminal].cursor_x, terminalArray[newTerminal].cursor_y);
    currTerminal = newTerminal;
    
    return 0;
}


void terminal_init(){
    int i; 
    currpid = 0;
    
    for(i = 0; i<MAX_TERMINALS; i++){
        processesid[i] = 1;    // SAYS PROCESS IS ACTIVE
        execute((const uint8_t *)("shell"));    //pid 0
        terminalArray[i].currRTC = 1024;
        // terminalArray[i].cur_PCB = ;
        // terminalArray[i].savedt_esp = ;
        // terminalArray[i].savedt_ebp = ;
        //terminalArray[i].terminalbuffer; 
        terminalArray[i].cursor_x = 0;
        terminalArray[i].cursor_y = 0;
        terminalArray[i].vidmemloc = VIDEO_T1 + FOUR_KILO_BYTE *i;
        currpid++;
    }

    currTerminal = 0;
}

